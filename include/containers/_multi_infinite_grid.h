#ifndef OX_LIB_MULTI_INFINITE_GRID_H
#define OX_LIB_MULTI_INFINITE_GRID_H

#include <cstdlib>
#include <vector>
#include <array>
#include <numeric>
#include <ranges>
#include <tuple>
#include <ox/array.h>
#include <ox/math.h>
#include "_2d_grid.h"
#include <optional>
#include <climits>

namespace ox {
    constexpr static auto default_hash_func = [](const auto& s) {
#ifdef __cpp_lib_format_ranges
        std::string ss = std::accumulate(s.begin(), s.end(), std::string(), [](const std::string& part, long l) {
            return part + std::to_string(l);
        });
        return std::hash<std::string>()(ss);
#elifdef __cpp_lib_ranges_join_with
        auto range = s | stdv::transform([](const auto& x) { return std::to_string(x); })
                   | stdv::join_with(std::string(",")) | stdv::common;
        auto ss = std::string(range.begin(), range.end());
        return std::hash<std::string>()(ss);
#endif
    };

    template <typename T, std::size_t Dimensions,
              typename Container = std::unordered_map<std::array<long, Dimensions>, T, decltype(default_hash_func)>>
    struct infinite_grid {
        static_assert(Dimensions > 0, "Dimensions of grid must be larger than zero");
    protected:
        using index_data = std::array<long, Dimensions>;
        constexpr static auto dimension_offsets = multi_dimensions<Dimensions>();
        Container data;
    public:
        using container = Container;
        using value_type = T;
        using iterator = typename container::iterator;
        using const_iterator = typename container::const_iterator;
        using size_type = std::size_t;

        template <typename... ContainerArgs>
        requires std::constructible_from<Container, ContainerArgs...>
        constexpr explicit infinite_grid(ContainerArgs... args) : data(args...){};

        template <typename... ContainerElems>
        constexpr explicit infinite_grid(ContainerElems... args) : data({args...}){};

        template <std::ranges::range R>
        requires std::constructible_from<Container, decltype(std::ranges::begin(std::declval<R>())),
                                         decltype(std::ranges::end(std::declval<R>()))>
        constexpr explicit infinite_grid(R&& r) : data(std::ranges::begin(r), std::ranges::end(r)){};

        constexpr infinite_grid(const std::initializer_list<T>& r)
        requires std::constructible_from<Container, std::initializer_list<T>>
                : data(r){};

        constexpr explicit infinite_grid(index_data dim, const std::initializer_list<T>& r) :
                infinite_grid(dim, std::views::all(r)){};

        constexpr infinite_grid() = default;

        constexpr typename Container::const_reference at(std::integral auto... l) const {
            auto bounds = pack_array<long>(l...);
            return data.at(bounds);
        }
        constexpr typename Container::reference at(std::integral auto... l) {
            auto bounds = pack_array<long>(l...);
            return data.at(bounds);
        }

#ifdef __cpp_multidimensional_subscript
        template <typename... Index>
        constexpr auto& operator[](Index... args) const {
            return data[pack_array(long(args)...)];
        }
        template <typename... Index>
        constexpr auto& operator[](Index... args) {
            return data[pack_array(long(args)...)];
        }
#endif

        constexpr auto& operator[](index_data i) const { return data[i]; }
        constexpr auto& operator[](index_data i) { return data[i]; }

        bool operator==(const infinite_grid& in) const = default;

        constexpr const Container& get_raw() const { return data; }

        Container& get_raw() { return data; }

#ifdef __cpp_lib_ranges_cartesian_product
        auto neighbour_range(index_data index) {
            auto curr_index = coord_from_index(index);
            constexpr auto return_size = ox::fast_pow(3zu, Dimensions) - 1;
            std::array<index_data, return_size> to_return;
            auto head = to_return.begin();
            for (auto offsets_t : std::apply(std::views::cartesian_product, dimension_offsets)) {
                auto offsets = array_from_tuple(offsets_t);
                if (std::all_of(offsets.begin(), offsets.end(), [](long l) { return l == 0; }))
                    continue;

                std::ranges::transform(offsets, curr_index, offsets.begin(), std::plus());
                *head++ = offsets;
            }
            return to_return;
        }

        auto cardinal_neighbour_range(index_data index) {
            auto curr_index = coord_from_index(index);
            std::array<index_data, 2 * Dimensions> to_return;
            auto head = to_return.begin();
            for (auto [offset, index] :
                 std::views::cartesian_product(std::array{-1l, 1l}, std::views::iota(0l, long(Dimensions)))) {
                auto new_index = curr_index;
                new_index[index] += offset;

                *head++ = new_index;
            }
            return to_return;
        }
#endif

        ox::grid2<T, Dimensions> to_finite_grid(T def = T{}) {
            if (data.empty())
                return {};
            std::pair<index_data, index_data> min_max = std::pair{data.begin()->first, data.begin()->first};
            for (auto key : data | stdv::keys) {
                stdr::transform(
                        key, min_max.first, min_max.first.begin(), [](long l1, long l2) { return std::min(l1, l2); });
                stdr::transform(
                        key, min_max.second, min_max.second.begin(), [](long l1, long l2) { return std::max(l1, l2); });
            }
            index_data dimensions;
            stdr::transform(min_max.second, min_max.first, dimensions.begin(), [](long high, long low) {
                return high - low + 1;
            });
            auto size = std::accumulate(dimensions.begin(), dimensions.end(), 1zu, std::multiplies());
            ox::grid2<T, Dimensions> to_return(dimensions, size, def);

            stdr::transform(min_max.first, dimensions.begin(), [](long l) { return -l; });
            to_return.set_center(dimensions);
            for (const auto& [key, value] : data) {
                to_return[key] = value;
            }

            return to_return;
        }
    };
} // namespace ox

#endif // OX_LIB_MULTI_INFINITE_GRID_H
