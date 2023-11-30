#ifndef OX_LIB__2D_GRID_H
#define OX_LIB__2D_GRID_H

#include <vector>
#include <concepts>
#include <span>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <functional>
#include "_multi_grid.h"
#include "ox/ranges.h"
#include "ox/algorithms.h"

namespace ox {
    template <typename T, typename Container = std::vector<T>>
    class grid : public grid2<T, 2, Container> {
    public:
        using grid2<T, 2, Container>::grid2;
        using grid2<T, 2, Container>::data;
        using grid2<T, 2, Container>::dimensions;
        using grid2<T, 2, Container>::center;
        using grid2<T, 2, Container>::get_size;
        template <typename BaseIterator>
        class row_iterator;

        using container = Container;
        using value_type = T;
        using raw_iterator = typename container::iterator;
        using const_raw_iterator = typename container::const_iterator;
        using iterator = row_iterator<raw_iterator>;
        using const_iterator = row_iterator<const_raw_iterator>;
        using size_type = std::size_t;

        template <typename... I>
        constexpr explicit grid(int _width, I... args) : grid2<T, 2, Container>({}, args...) {
            set_width(_width);
        };

        template <std::ranges::range R,
                  std::invocable<typename std::remove_reference_t<R>::value_type::value_type> Proj>
        requires std::ranges::range<typename std::remove_reference_t<R>::value_type>
              && requires(typename std::remove_reference_t<R>::value_type row,
                          typename std::remove_reference_t<R>::value_type::value_type elem, Container c, Proj p) {
                     { c.push_back(std::invoke(p, elem)) };
                     { row.size() };
                 }
        constexpr explicit grid(R&& r, Proj p) {
            for (const auto& row : r) {
                this->dimensions[0] = row.size();
                std::transform(row.begin(), row.end(), std::back_inserter(data), p);
            }
            dimensions[1] = data.size() / dimensions[0];
        };

        template <std::ranges::range R>
        requires std::ranges::range<typename R::value_type>
              && requires(typename std::remove_reference_t<R>::value_type row,
                          typename std::remove_reference_t<R>::value_type::value_type elem, Container c) {
                     { c.push_back(elem) };
                     { row.size() };
                 }
        constexpr explicit grid(R&& r) : grid(std::forward<R>(r), std::identity()){};

        template <std::ranges::range R,
                  std::invocable<typename std::remove_reference_t<R>::value_type::value_type> Proj>
        requires std::ranges::range<typename std::remove_reference_t<R>::value_type> && std::is_aggregate_v<Container>
        constexpr explicit grid(R&& r, Proj p) {
            for (auto& row : r) {
                this->dimensions[0] = row.size();
                std::transform(row.begin(), row.end(), data.begin(), p);
            }
        };

        template <std::ranges::range R>
        requires std::ranges::range<typename std::remove_reference_t<R>::value_type> && std::is_aggregate_v<Container>
        constexpr explicit grid(R&& r) : grid(std::forward(r), std::identity()){};

        constexpr void set_width(int new_width) {
            assert(data.size() % new_width == 0);
            dimensions[0] = new_width;
            dimensions[1] = data.size() / new_width;
        }

        [[nodiscard]] constexpr size_type get_height() const { return dimensions[1]; }

        [[nodiscard]] constexpr size_type get_width() const { return dimensions[0]; }

        [[nodiscard]] constexpr std::pair<size_type, size_type> get_dimensions() const {
            return {dimensions[0], dimensions[1]};
        }

        template <std::invocable<T&> I, std::invocable<> O>
        void leveled_foreach(I inner, O outer) const {
            ox::nested_foreach(begin(), end(), outer, inner);
            return;
            for (auto row : *this) {
                for (const T& item : row) {
                    inner(item);
                }
                outer();
            }
        }

        template <std::invocable<raw_iterator&> I, std::invocable<> O>
        void leveled_iterators(I inner, O outer) const {
            for (auto row : *this) {
                auto start = row.begin();
                auto end = row.end();
                for (; start != end; ++start) {
                    inner(start);
                }
                outer();
            }
        }
        const_iterator begin() const { return const_iterator(dimensions[0], data.begin()); }
        const_iterator end() const { return const_iterator(dimensions[0], data.end()); }

        iterator begin() { return iterator(dimensions[0], data.begin()); }
        iterator end() { return iterator(dimensions[0], data.end()); }

        [[nodiscard]] std::optional<raw_iterator> up(std::optional<raw_iterator> curr) const {
            if (!curr || (*curr - get_width()) < data.begin())
                return std::nullopt;
            return *curr - get_width();
        }
        [[nodiscard]] std::optional<raw_iterator> left(std::optional<raw_iterator> curr) const {
            if (!curr || (*curr - data.begin()) % get_width() == 0)
                return std::nullopt;
            return *curr - 1;
        }
        [[nodiscard]] std::optional<raw_iterator> right(std::optional<raw_iterator> curr) const {
            if (!curr || (*curr - data.begin() + 1) % get_width() == 0)
                return std::nullopt;
            return *curr + 1;
        }
        [[nodiscard]] std::optional<raw_iterator> down(std::optional<raw_iterator> curr) const {
            if (!curr || (*curr + get_width()) >= data.end())
                return std::nullopt;
            return *curr + get_width();
        }

        [[nodiscard]] std::optional<const_raw_iterator> up(std::optional<const_raw_iterator> curr) const {
            if (!curr || (*curr - get_width()) < data.begin())
                return std::nullopt;
            return *curr - get_width();
        }
        [[nodiscard]] std::optional<const_raw_iterator> left(std::optional<const_raw_iterator> curr) const {
            if (!curr || (*curr - data.begin()) % get_width() == 0)
                return std::nullopt;
            return *curr - 1;
        }
        [[nodiscard]] std::optional<const_raw_iterator> right(std::optional<const_raw_iterator> curr) const {
            if (!curr || (*curr - data.begin() + 1) % get_width() == 0)
                return std::nullopt;
            return *curr + 1;
        }
        [[nodiscard]] std::optional<const_raw_iterator> down(std::optional<const_raw_iterator> curr) const {
            if (!curr || (*curr + get_width()) >= data.end())
                return std::nullopt;
            return *curr + get_width();
        }

        auto neighbour_range(const_raw_iterator current) {
            std::array neighbours{up(current),
                                  up(left(current)),
                                  left(current),
                                  down(left(current)),
                                  down(current),
                                  down(right(current)),
                                  right(current),
                                  up(right(current))};
            return neighbours;
        }
        auto cardinal_neighbour_range(const_raw_iterator current) {
            std::array neighbours{up(current), left(current), down(current), right(current)};
            return neighbours;
        }

        auto neighbour_range(raw_iterator current) {
            std::array neighbours{up(std::optional(current)),
                                  up(left(std::optional(current))),
                                  left(std::optional(current)),
                                  down(left(std::optional(current))),
                                  down(std::optional(current)),
                                  down(right(std::optional(current))),
                                  right(std::optional(current)),
                                  up(right(std::optional(current)))};
            return neighbours;
        }
        auto cardinal_neighbour_range(raw_iterator current) {
            std::array neighbours{up(std::optional(current)),
                                  left(std::optional(current)),
                                  down(std::optional(current)),
                                  right(std::optional(current))};
            return neighbours;
        }

        static auto valid_index() {
            return std::views::filter([](std::optional<raw_iterator> x) { return x.has_value(); })
                 | std::views::transform([](std::optional<raw_iterator> x) { return *x; });
        }

        static auto const_valid_index() {
            return std::views::filter([](std::optional<const_raw_iterator> x) { return x.has_value(); })
                 | std::views::transform([](std::optional<const_raw_iterator> x) { return *x; });
        }
    };

    template <typename T, typename Container>
    template <typename BaseIterator>
    class grid<T, Container>::row_iterator {
    public:
        class reference_proxy : public std::ranges::subrange<BaseIterator, BaseIterator> {
            using std::ranges::subrange<BaseIterator, BaseIterator>::subrange;

            reference_proxy& operator=(const std::ranges::range auto& range) const {
                std::ranges::copy_n(range, get_size(), begin());
            }
        };
        using iterator_category = std::random_access_iterator_tag;
        using value_type = reference_proxy;
        using difference_type = int;
        using pointer = reference_proxy*;
        using const_pointer = const reference_proxy*;
        using referece = reference_proxy&;
        using const_reference = const reference_proxy&;
    private:
        int width;
        value_type current_data;
    public:
        row_iterator() : width{0}, current_data(){};

        row_iterator(int _width, BaseIterator _curr) : width(_width), current_data{_curr, _curr + width} {}

        const_reference operator*() const { return current_data; }
        const_pointer operator->() const { return &current_data; }
        row_iterator& operator++() {
            current_data = value_type{current_data.end(), current_data.end() + width};
            return *this;
        }
        row_iterator operator++(int) {
            row_iterator cpy(*this);
            ++(*this);
            return cpy;
        }
        row_iterator operator+(int i) const {
            row_iterator cpy(*this);
            cpy.current_data = value_type{current_data.begin() + width * i, current_data.end() + width * i};
            return cpy;
        }
        row_iterator& operator+=(int i) {
            current_data = value_type{current_data.begin() + width * i, current_data.end() + width * i};
            return *this;
        }
        row_iterator& operator--() const {
            current_data = value_type{current_data.begin() - width, current_data.begin()};
            current_data.first -= width;
            return *this;
        }
        row_iterator operator--(int) {
            row_iterator cpy(*this);
            --(*this);
            return cpy;
        }
        row_iterator operator-(int i) const {
            row_iterator cpy(*this);
            cpy.current_data = value_type{current_data.begin() - width * i, current_data.end() - width * i};
            return cpy;
        }
        int operator-(const row_iterator& other) const {
            return (current_data.begin() - other.current_data.begin()) / width;
        }
        row_iterator& operator-=(int i) {
            current_data = value_type{current_data.begin() - width * i, current_data.end() - width * i};
            return *this;
        }
        row_iterator operator[](int i) const { return *this + i; }
        auto operator<=>(const row_iterator& other) const {
            return current_data.begin() <=> other.current_data.begin();
        };
        bool operator==(const row_iterator& other) const { return current_data.begin() == other.current_data.begin(); }
        bool operator!=(const row_iterator& other) const { return current_data.begin() != other.current_data.begin(); }
    };
} // namespace ox

#endif // OX_LIB__2D_GRID_H
