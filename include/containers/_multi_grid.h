#ifndef OX_LIB_MULTI_GRID_H
#define OX_LIB_MULTI_GRID_H

#include <cstdlib>
#include <vector>
#include <array>
#include <numeric>
#include <ranges>
#include <tuple>
#include <ox/array.h>
#include <ox/math.h>
#include <optional>

namespace ox {

    template <std::size_t Dimensions>
    constexpr std::array<std::array<long, 3>, Dimensions> multi_dimensions() {
        std::array<std::array<long, 3>, Dimensions> to_return;
        std::array offsets{-1l, 0l, 1l};
        std::fill(to_return.begin(), to_return.end(), offsets);
        return to_return;
    };

    template <typename T, std::size_t Dimensions, typename Container = std::vector<T>>
    struct grid2 {
        static_assert(Dimensions > 0, "Dimensions of grid must be larger than zero");
    protected:
        using index_data = std::array<long, Dimensions>;
        constexpr static auto dimension_offsets = multi_dimensions<Dimensions>();
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
            return std::accumulate(widths.begin(), widths.end(), 0l);
        }

        constexpr bool inbounds(index_data x) const {
            x = to_absolute_index(x);
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

        constexpr void set_dimensions(std::integral auto... l) { set_dimensions(pack_array<long>(l...)); }

        constexpr long get_dimension(size_t index) { return dimensions[index]; }

        constexpr index_data get_dimensions() { return dimensions; }

        constexpr auto get_size() const { return data.size(); }

        constexpr typename Container::const_reference at(std::integral auto... l) const {
            auto bounds = pack_array<long>(l...);
            check_bounds_relative(bounds);
            return data.at(get_base_index(bounds));
        }
        constexpr typename Container::reference at(std::integral auto... l) {
            auto bounds = pack_array<long>(l...);
            check_bounds_relative(bounds);
            return data.at(get_base_index(bounds));
        }

        constexpr typename std::optional<typename Container::value_type> get(std::integral auto... l) const {
            auto bounds = pack_array<long>(l...);
            if (!inbounds(bounds))
                return std::nullopt;
            return data[get_base_index(bounds)];
        }

#ifdef __cpp_multidimensional_subscript
        template <typename... Index>
        constexpr typename Container::const_reference operator[](Index... args) const { return data[get_base_index(pack_array(args...))]; }
        template <typename... Index>
        constexpr typename Container::reference operator[](Index... args) { return data[get_base_index(pack_array(args...))]; }
#endif

        constexpr typename Container::const_reference operator[](int i) const { return data[i]; }
        constexpr typename Container::reference operator[](int i) { return data[i]; }

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

#ifdef __cpp_lib_ranges_cartesian_product
        template <typename Pointer>
        requires(std::is_same_v<std::remove_cv_t<Pointer>, const_raw_iterator>
                 || std::is_same_v<std::remove_cv_t<Pointer>, raw_iterator>)
        auto neighbour_range(Pointer index) {
            auto curr_index = coord_from_index(index);
            constexpr auto return_size = ox::fast_pow(3zu, Dimensions) - 1;
            std::array<std::optional<Pointer>, return_size> to_return;
            auto head = to_return.begin();
            for (auto offsets_t : std::apply(std::views::cartesian_product, dimension_offsets)) {
                auto offsets = array_from_tuple(offsets_t);
                if (std::all_of(offsets.begin(), offsets.end(), [](long l) { return l == 0; }))
                    continue;

                std::ranges::transform(offsets, curr_index, offsets.begin(), std::plus());
                if (!inbounds(offsets)) {
                    *head++ = std::nullopt;
                } else {
                    *head++ = data.begin() + get_base_index(offsets);
                }
            }
            return to_return;
        }

        template <typename Pointer>
        requires(std::is_same_v<std::remove_cv_t<Pointer>, const_raw_iterator>
                 || std::is_same_v<std::remove_cv_t<Pointer>, raw_iterator>)
        auto cardinal_neighbour_range(Pointer index) {
            auto curr_index = coord_from_index(index);
            std::array<std::optional<Pointer>, 2 * Dimensions> to_return;
            auto head = to_return.begin();
            for (auto [offset, index] :
                 std::views::cartesian_product(std::array{-1l, 1l}, std::views::iota(0l, long(Dimensions)))) {
                auto new_index = curr_index;
                new_index[index] += offset;

                if (!inbounds(new_index)) {
                    *head++ = std::nullopt;
                } else {
                    *head++ = data.begin() + get_base_index(new_index);
                }
            }
            return to_return;
        }
#endif

#undef NLongConcept
    };
} // namespace ox

#endif // OX_LIB_MULTI_GRID_H
