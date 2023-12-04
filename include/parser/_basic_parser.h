

#ifndef OXLIBCODE_BASIC_PARSER
#define OXLIBCODE_BASIC_PARSER

#include <vector>
#include <utility>
#include <charconv>
#include <cctype>
#include <expected>
#include <any>
#include <concepts>
#include <functional>

namespace ox::parser {
    enum class ParseError {
        bad = 1,
    };

    inline bool debug = false;
    inline long _indent = 0;
    inline std::ostream& indent(std::ostream& os) {
        for (long i = 0; i < 4 * _indent; ++i) {
            os << ' ';
        }
        return os;
    }

    using parse_result = std::expected<std::pair<long, std::any>, ParseError>;

#define PARSE_HEADER         [[nodiscard]] parse_result parse(void* ref, std::string_view s) const override
#define PARSE_VIRTUAL_HEADER [[nodiscard]] virtual parse_result parse(void*, std::string_view) const

    template <typename Tuple, int Index>
    void _parse_any_vector_imp(const std::vector<std::any>&, Tuple&) {}
    template <typename Tuple, int Index, typename T, typename... Rest>
    void _parse_any_vector_imp(const std::vector<std::any>& vec, Tuple& t) {
        std::get<Index>(t) = std::any_cast<T>(vec[Index]);
        _parse_any_vector_imp<Tuple, Index + 1, Rest...>(vec, t);
    }
    template <typename... T>
    std::tuple<T...> parse_any_vector(const std::vector<std::any>& vec) {
        std::tuple<T...> to_return;
        _parse_any_vector_imp<std::tuple<T...>, 0, T...>(vec, to_return);
        return to_return;
    }

    class Parser;
    using ParserPoint = std::unique_ptr<Parser>;
    class Parser {
    public:
        PARSE_VIRTUAL_HEADER = 0;
        virtual ~Parser() = default;
    };

    class Literal final : public Parser {
        std::string_view match;
        std::function<std::any(void*, std::string_view)> callback;
    public:
        explicit Literal(const char* _m) :
                match{_m}, callback{[](auto, auto x) {
                    return x;
                }} {};
        template <std::invocable<void*> Func>
        explicit Literal(const char* _m, const Func& callback) : match{_m}, callback{callback} {};

        PARSE_HEADER {
            if (debug) {
                std::cout << indent << "Parsing Literal " << std::quoted(match) << " in " << std::quoted(s)
                          << std::endl;
            }
            size_t index = s.find(match);
            if (index == std::string_view::npos) {
                if (debug) {
                    std::cout << indent << "Literal FAILED" << std::endl;
                }
                return std::unexpected(ParseError::bad);
            }
            for (int x = 0; size_t(x) < index; ++x) {
                if (!isspace(s[x])) {
                    return std::unexpected(ParseError::bad);
                }
            }
            if (debug)
                std::cout << indent << "FOUND \033[31m" << std::quoted(s.substr(0, index + match.size())) << "\033[0m"
                          << std::endl;
            auto x = callback(ref, s.substr(index, match.size()));
            return std::pair{long(index + match.size()), x};
        }
        ~Literal() override = default;
    };
    namespace literals {
        inline auto operator""_l(const char* c, size_t) {
            return Literal(c);
        }
    } // namespace literals

    class Int : public Parser {
        std::function<std::any(void*, long)> callback;
    public:
        Int() : callback([](auto, long l) { return l; }){};

        template <std::invocable<void*, long> Func>
        Int(const Func& _callback) : callback(_callback){};
    public:
        PARSE_HEADER {
            if (debug)
                std::cout << indent << "Parsing Int in " << std::quoted(s) << std::endl;
            long l;
            const char* head = s.begin();
            while (isspace(*head) && head < s.end())
                ++head;
            auto [end, err] = std::from_chars(head, s.end(), l);
            if (err != std::errc{}) {
                if (debug)
                    std::cout << indent << "FAILED TO PARSE INT" << std::endl;
                return std::unexpected(ParseError::bad);
            }
            if (debug)
                std::cout << indent << "FOUND \033[31m" << std::quoted(s.substr(0, end - s.begin())) << "\033[0m"
                          << std::endl;
            callback(ref, l);
            return std::pair{end - s.begin(), std::any(l)};
        };
    };

    enum class ListEnding { unended, ended, either };

    class List : public Parser {
        std::string_view delimiter;
        ParserPoint repeat;
        std::function<std::any(void*)> callback;
        ListEnding tail;
    public:
        List(List&&) = default;

        template <std::derived_from<Parser> ParserSub>
        explicit List(const char* _delim, ParserSub&& _rep, ListEnding _t = ListEnding::unended) :
                delimiter{_delim},
                repeat(std::make_unique<ParserSub>(std::move(_rep))),
                callback([](void*) { return nullptr; }),
                tail(_t){};

        template <std::derived_from<Parser> ParserSub>
        explicit List(ParserSub&& _rep, ListEnding _t = ListEnding::unended) :
                delimiter{""},
                repeat(std::make_unique<ParserSub>(std::move(_rep))),
                callback([](void*) { return nullptr; }),
                tail(_t){};

        template <std::derived_from<Parser> ParserSub, std::invocable<void*> Func>
        explicit List(const char* _delim, ParserSub&& _rep, const Func& _callback,
                      ListEnding _t = ListEnding::unended) :
                delimiter{_delim},
                repeat(std::make_unique<ParserSub>(std::move(_rep))),
                callback(_callback),
                tail(_t){};

        parse_result parse_with_delim(void* ref, std::string_view s) const {
            if (debug)
                std::cout << indent << "Parsing List with delim " << std::quoted(delimiter) << " in " << std::quoted(s)
                          << std::endl;
            size_t pos = 0;
            size_t old_pos = 0;
            long size = 0;
            ++_indent;
            while ((pos = s.find(delimiter, old_pos)) != std::string_view::npos) {
                callback(ref);
                std::string_view sub = s.substr(old_pos, pos - old_pos);
                auto subsize = repeat->parse(ref, sub);
                if (!subsize) {
                    --_indent;
                    if (debug)
                        std::cout << indent << "Failed to parse LIST" << std::endl;
                    return subsize;
                }
                size += subsize->first + delimiter.size();
                old_pos = pos + delimiter.size();
            }
            if (tail == ListEnding::unended || (tail == ListEnding::either && old_pos != (s.size()))) {
                std::string_view sub = s.substr(old_pos, s.end() - s.begin());
                callback(ref);
                auto subsize = repeat->parse(ref, sub);
                if (!subsize) {
                    --_indent;
                   return subsize;
                }
                size += subsize->first;
            }
            --_indent;
            if (debug)
                std::cout << indent << "FOUND \033[31m" << std::quoted(s.substr(0, size)) << "\033[0m" << std::endl;
            return std::pair{size, std::any(nullptr)};
        }

        parse_result parse_without_delim(void* ref, std::string_view s) const {
            if (debug)
                std::cout << indent << "Parsing List without delim in " << std::quoted(s) << std::endl;
            size_t pos = 0;
            long size = 0;
            ++_indent;
            while (true) {
                callback(ref);
                std::string_view sub = s.substr(pos);
                auto subsize = repeat->parse(ref, sub);
                if (!subsize) {
                    if (debug)
                        std::cout << indent << "Failed to parse LIST" << std::endl;
                    break;
                }
                size += subsize->first;
                pos += subsize->first;
            }
            --_indent;
            if (debug)
                std::cout << indent << "FOUND \033[31m" << std::quoted(s.substr(0, size)) << "\033[0m" << std::endl;
            return std::pair{size, std::any(nullptr)};
        }

        PARSE_HEADER {
            if (delimiter.empty())
                return parse_without_delim(ref, s);
            return parse_with_delim(ref, s);
        }

        ~List() override = default;
    };

    template <typename T>
    class _list : public Parser {
    protected:
        std::vector<ParserPoint> parts;
        void add_args() {}

        template <std::derived_from<Parser> ParserSum, typename... ARGS>
        void add_args(ParserSum&& p, ARGS&&... rest) {
            parts.emplace_back(new ParserSum(std::move(p)));
            add_args(std::forward<ARGS>(rest)...);
        }
    public:
        template <typename... ARGS>
        explicit _list(ARGS&&... rest) {
            add_args(std::forward<ARGS>(rest)...);
        };

        _list(_list&&) = default;
        ~_list() override = default;
    };

    class Combination : public _list<Combination> {
        template <std::derived_from<Parser> ParserSub>
        friend Combination operator+(Combination&& l, ParserSub&& r);
        friend Combination operator+(Parser&& l, Parser&& r);

        std::function<std::any(void*, std::vector<std::any>)> callback;
    public:
        template <std::invocable<void*, std::vector<std::any>> Func, typename... T>
        Combination(Func f, T... t) : _list<Combination>(std::forward<T>(t)...), callback(f) {}

        template <typename... T>
        explicit Combination(T&&... t) :
                _list<Combination>(std::forward<T>(t)...), callback([](auto, auto) { return nullptr; }) {}

        Combination(Combination&&) = default;

        template <std::invocable<void*, std::vector<std::any>> Func>
        void setCallback(Func f) {
            callback = f;
        }

        PARSE_HEADER {
            if (debug)
                std::cout << indent << "Parsing Combination in " << std::quoted(s) << std::endl;
            ++_indent;
            auto subpart = s;
            long pos = 0;
            std::vector<std::any> subres;
            for (const auto& part : this->parts) {
                parse_result read = part->parse(ref, subpart);
                if (!read) {
                    --_indent;
                    if (debug)
                        std::cout << indent << "Failed to Parse Combination" << std::endl;
                    return read;
                }
                subres.push_back(read->second);
                subpart = subpart.substr(read->first);
                pos += read->first;
            }
            --_indent;
            if (debug)
                std::cout << indent << "FOUND \033[31m" << std::quoted(s.substr(0, pos)) << "\033[0m" << std::endl;
            auto x = callback(ref, subres);
            return std::pair{pos, x};
        }

        ~Combination() override = default;
    };

    class Or : public _list<Or> {
        using _list<Or>::_list;
        friend Or operator|(Parser&& l, Parser&& r);
        template <std::derived_from<Parser> ParserSub>
        friend Or operator|(Or&& l, ParserSub&& r);
    public:
        PARSE_HEADER {
            if (debug)
                std::cout << indent << "Parsing Combination in " << std::quoted(s) << std::endl;
            ++_indent;
            for (const auto& part : this->parts) {
                auto read = part->parse(ref, s);
                if (!read)
                    continue;
                --_indent;
                if (debug)
                    std::cout << indent << "FOUND \033[31m" << std::quoted(s.substr(0, read->first)) << "\033[0m"
                              << std::endl;
                return read;
            }
            --_indent;
            if (debug)
                std::cout << indent << "FAILED to find OR" << std::endl;
            return std::unexpected(ParseError::bad);
        };
    };

    template <std::derived_from<Parser> ParserSub1, std::derived_from<Parser> ParserSub2>
    requires(!std::is_same_v<std::remove_reference_t<std::remove_cv_t<ParserSub1>>, Or>)
    Or operator|(ParserSub1&& l, ParserSub2&& r) {
        return Or{std::move(l), std::move(r)};
    }

    template <std::derived_from<Parser> ParserSub>
    Or operator|(Or&& l, ParserSub&& r) {
        l.parts.emplace_back(new ParserSub(std::move(r)));
        return std::move(l);
    }

    template <std::derived_from<Parser> ParserSub1, std::derived_from<Parser> ParserSub2>
    Combination operator+(ParserSub1&& l, ParserSub2&& r) {
        return Combination{std::move(l), std::move(r)};
    }

    template <std::derived_from<Parser> ParserSub>
    Combination operator+(Combination&& l, ParserSub&& r) {
        l.parts.emplace_back(new ParserSub(std::move(r)));
        return std::move(l);
    }
} // namespace ox::parser

/*
 * GAME = "Game"_l + Int();
 * COLOR = "red"_l | "blue"_l | "green"_l;
 * GAME = = INT() + COLOR;
 * PULL = List(",", GAME);
 * RESULTS = List(";", PULL);
 * FULL_GAME = GAME + ": "_l + RESULTS;
 */

#endif // ADVENTOFCODE_BASIC_PARSER
