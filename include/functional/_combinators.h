

#ifndef OXLIB_COMBINATORS_IMP_H
#define OXLIB_COMBINATORS_IMP_H

#include <functional>
#include <concepts>

namespace ox {

#define DEFINE_COMBINATOR(Func) constexpr inline auto Func = Func##_t();

    auto binary(const auto& f) {
        return [f](const auto x, const auto& y) {
            return f(x)(y);
        };
    }

    // const I = x => x;
    struct Identity_t {
        constexpr auto operator()(const auto& x) const { return x; }
    };
    DEFINE_COMBINATOR(Identity);

    // const B = f => g => x => f(g(x));
    struct BlueBird_t {
        constexpr auto operator()(const auto& f) const {
            return [f](const auto& g) {
                return [f, g](const auto& x) {
                    return f(g(x));
                };
            };
        }
        constexpr auto operator()(const auto& f, const auto& g) const {
            return [f, g](const auto& x) {
                return f(g(x));
            };
        }
    };
    DEFINE_COMBINATOR(BlueBird);

    // const M = f => f(f);
    struct MockingBird_t {
        constexpr auto operator()(const auto& f) const { return f(f); }
    };
    DEFINE_COMBINATOR(MockingBird);

    // const M = a => b => a;
    struct Kestrel_t {
        constexpr auto operator()(const auto& a) const {
            return [a](const auto&) {
                return a;
            };
        }
        constexpr auto operator()(const auto& a, const auto&) const { return a; }
    };
    DEFINE_COMBINATOR(Kestrel);

    // const KI = a => b => b;
    struct Kite_t {
        constexpr auto operator()(const auto&) const {
            return [](const auto& b) {
                return b;
            };
        }

        constexpr auto operator()(const auto&, const auto& b) const { return b; }
    };
    DEFINE_COMBINATOR(Kite);

    // const C = f => a => b => f(b)(a)
    struct Cardinal_t {
        template <typename Func>
        struct CardinalInternal {
            Func f;
            auto operator()(const auto& a) {
                return [this, a](const auto& b) {
                    return f(b)(a);
                };
            };
            auto operator()(const auto& a, const auto& b) { return f(b, a); };
        };

        constexpr auto operator()(const auto& f) const { return CardinalInternal(f); }
    };
    DEFINE_COMBINATOR(Cardinal);



    // const B = f => g => x => f(g(x));
    struct Starling_t {
        constexpr auto operator()(const auto& f) const {
            return [f](const auto& g) {
                return [f, g](const auto& x) {
                    return f(x)(g(x));
                };
            };
        }
        constexpr auto operator()(const auto& f, const auto& g) const {
            return [f, g](const auto& x) {
                return f(x, g(x));
            };
        }
    };
    DEFINE_COMBINATOR(Starling);



    namespace combinators {
        constexpr inline auto& I = Identity;
        constexpr inline auto& B = BlueBird;
        constexpr inline auto& C = Cardinal;
        constexpr inline auto& K = Kestrel;
        constexpr inline auto& KI = Kite;
        constexpr inline auto& S = Starling;
    } // namespace combinators
#undef DEFINE_COMBINATOR
} // namespace ox

#endif // OXLIB_COMBINATORS_H
