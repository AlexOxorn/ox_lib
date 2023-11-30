#ifndef OX_LIB_MULTI_GRID_H
#define OX_LIB_MULTI_GRID_H

#include <cstdlib>
#include <vector>
#include <array>
#include <numeric>
#include <ranges>

namespace ox {
    template <typename T, typename... Args>
    requires(std::is_convertible_v<Args, T> && ...)
    auto _pack_array(T first, Args... rest) {
        return std::array<T, sizeof...(Args) + 1>{first, static_cast<T>(rest)...};
    }

    template <typename T, std::size_t Dimensions, typename Container = std::vector<T>>
    struct grid2 {
        static_assert(Dimensions > 0, "Dimensions of grid must be larger than zero");
    protected:
        using index_data = std::array<long, Dimensions>;
        Container data;
        index_data dimensions;
        index_data center{};

        index_data to_absolute_index(index_data x) const {
            std::ranges::transform(x, center, x.begin(), std::plus<>());
            return x;
        }

        index_data to_relative_index(index_data x) const {
            std::ranges::transform(x, center, x.begin(), std::minus<>());
            return x;
        }

        [[nodiscard]] index_data pseudo_width() const {
            index_data widths{1};
            std::partial_sum(dimensions.begin(), dimensions.end() - 1, widths.begin() + 1, std::multiplies<>());
            return widths;
        }

        long get_base_index(index_data x) const {
            index_data widths = pseudo_width();
            index_data abs = to_absolute_index(x);
            std::ranges::transform(abs, widths, widths.begin(), std::multiplies<>());
            return std::accumulate(widths.begin(), widths.end(), 0l);;
        }

        constexpr bool inbounds(index_data x) const {
#if 0 && defined(__cpp_lib_ranges_zip)
            for (auto [index, bound] : std::views::zip(x, dimensions)) {
                if (index < 0 || index >= bound) {
                    return false;
                }
            }
#else
            for (long i = 0; i < long(Dimensions); ++i) {
                if (x[i] < 0 || x[i] >= dimensions[i]) {
                    return false;
                }
            }
#endif
            return true;
        }

        void check_bounds_absolute(index_data x) const {
            if (!inbounds(x)) {
                throw std::out_of_range("Index out of range");
            }
        }

        void check_bounds_relative(index_data x) const {
            auto abs = to_absolute_index(x);
            return check_bounds_absolute(abs);
        }
    public:
        template <typename BaseIterator>
        class row_iterator;

        using container = Container;
        using value_type = T;
        using raw_iterator = typename container::iterator;
        using const_raw_iterator = typename container::const_iterator;
        using iterator = row_iterator<raw_iterator>;
        using const_iterator = row_iterator<const_raw_iterator>;
        using size_type = std::size_t;

        template <typename... ContainerArgs>
        requires std::constructible_from<Container, ContainerArgs...>
        constexpr explicit grid2(index_data dim, ContainerArgs... args) : data(args...), dimensions(dim){};

        template <typename... ContainerElems>
        constexpr explicit grid2(index_data dim, ContainerElems... args) : data({args...}), dimensions(dim){};

        template <std::ranges::range R>
        requires std::constructible_from<Container, decltype(std::ranges::begin(std::declval<R>())),
                                         decltype(std::ranges::end(std::declval<R>()))>
        constexpr explicit grid2(index_data dim, R&& r) :
                data(std::ranges::begin(r), std::ranges::end(r)), dimensions(dim){};

        constexpr explicit grid2(index_data dim, const std::initializer_list<T>& r)
        requires std::constructible_from<Container, std::initializer_list<T>>
                : data(r), dimensions(dim){};

        constexpr explicit grid2(index_data dim, const std::initializer_list<T>& r) : grid2(dim, std::views::all(r)){};

        template <std::ranges::range R>
        constexpr explicit grid2(index_data dim, R&& r) : dimensions(dim) {
            if constexpr (std::ranges::random_access_range<R>) {
                data.reserve(std::ranges::distance(r));
            }
            std::copy(std::ranges::begin(r), std::ranges::end(r), std::back_inserter(data));
        };

        constexpr grid2(index_data dim)
        requires std::constructible_from<Container, long>
                : dimensions(dim), data(std::accumulate(dim.begin(), dim.end(), 1, std::multiplies<>())){};

        constexpr grid2() : data(), dimensions(){};

        constexpr void set_dimensions(index_data data) { dimensions = data; }

        constexpr void set_dimensions(std::integral auto... l) { set_dimensions(_pack_array<long>(l...)); }

        constexpr long get_dimension(size_t index) { return dimensions[index]; }

        constexpr index_data get_dimensions() { return dimensions; }

        constexpr auto get_size() const { return data.size(); }

        constexpr typename Container::const_reference at(std::integral auto... l) const {
            auto bounds = _pack_array<long>(l...);
            check_bounds_relative(bounds);
            return data.at(get_base_index(bounds));
        }
        constexpr typename Container::reference at(std::integral auto... l) {
            auto bounds = _pack_array<long>(l...);
            check_bounds_relative(bounds);
            return data.at(get_base_index(bounds));
        }

        constexpr typename std::optional<typename Container::value_type> get(std::integral auto... l) const {
            auto bounds = _pack_array<long>(l...);
            if (!inbounds(bounds))
                return std::nullopt;
            return data[get_base_index(bounds)];
        }

#ifdef __cpp_multidimensional_subscript

#endif

        constexpr typename Container::const_reference operator[](int i) { return data[i]; }
        constexpr typename Container::reference operator[](int i) const { return data[i]; }

        constexpr auto operator<=>(const grid2& other) const
        requires std::three_way_comparable<Container>
        {
            return data <=> other.data;
        }
        constexpr bool operator==(const grid2& other) const
        requires std::equality_comparable<Container>
        {
            return data == other.data;
        }

#ifndef _LIBCPP_VERSION
        constexpr auto operator<=>(const grid2& other) const
        requires(!std::three_way_comparable<Container> && std::three_way_comparable<T>)
        {
            return std::lexicographical_compare_three_way(
                    data.begin(), data.end(), other.data.begin(), other.data.end());
        }
#else
        constexpr auto operator<(const grid2& other) const
        requires(std::three_way_comparable<T>)
        {
            return std::lexicographical_compare(data.begin(), data.end(), other.data.begin(), other.data.end());
        }
        constexpr auto operator<=(const grid2& other) const
        requires(std::three_way_comparable<T>)
        {
            return *this < other || *this == other;
        }
        constexpr auto operator>=(const grid2& other) const
        requires(std::three_way_comparable<T>)
        {
            return !(*this < other);
        }
        constexpr auto operator>(const grid2& other) const
        requires(std::three_way_comparable<T>)
        {
            return *this != other && *this >= other;
        }
#endif
        constexpr bool operator==(const grid2& other) const
        requires(!std::equality_comparable<Container> && std::equality_comparable<T>)
        {
            auto [it1, it2] = std::mismatch(data.begin(), data.end(), other.begin(), other.end());
            return it1 == data.end() && it2 == data.end();
        }

        constexpr const Container& get_raw() const { return data; }

        Container& get_raw() { return data; }

        [[nodiscard]] index_data coord_from_index(long index) const {
            /*
             *   x  =  index / 1              % width
             *   y  =  index / width          % length
             *   z  =  index / width * length % depth
             */
            index_data widths = pseudo_width();
            std::ranges::transform(widths, widths.begin(), [index](long w) { return index / w; });
            std::ranges::transform(widths, dimensions, widths.begin(), [](long w, long dim) { return w % dim; });
            return widths;
        }

        [[nodiscard]] index_data coord_from_index(const_raw_iterator index) const {
            long i = index - data.begin();
            return coord_from_index(i);
        }

#undef NLongConcept
    };
} // namespace ox

#endif // OX_LIB_MULTI_GRID_H
