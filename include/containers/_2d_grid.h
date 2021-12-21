//
// Created by alexoxorn on 2021-12-16.
//

#ifndef OX_LIB__2D_GRID_H
#define OX_LIB__2D_GRID_H

#include <vector>
#include <concepts>
#include <span>
#include <cassert>
#include <algorithm>
#include "ox/ranges.h"

namespace ox {
    template <typename T, typename Container = std::vector<T>>
    class grid {
    protected:
        Container data;
        int width;
    public:
        class row_iterator;

        using container = Container;
        using value_type = T;
        using raw_iterator = typename container::iterator;
        using const_raw_iterator = typename container::const_iterator;
        using iterator = row_iterator;

        template <typename ...I>
        explicit grid(int _width, I... args) : data(args...), width(_width) {};

        template <std::ranges::range R>
        requires std::constructible_from<Container, typename std::remove_reference_t<R>::iterator, typename R::iterator>
        explicit grid(int _width, R&& r) : data(r.begin, r.end()), width(_width) {};

        template <std::ranges::range R>
        explicit grid(int _width, R&& r) : width(_width) {
            std::copy(r.begin, r.end, data.begin());
        };

        template <std::ranges::range R, std::invocable<typename std::remove_reference_t<R>::value_type::value_type> Proj>
        requires std::ranges::range<typename std::remove_reference_t<R>::value_type>
                 && requires(
                         typename std::remove_reference_t<R>::value_type row,
                         typename std::remove_reference_t<R>::value_type::value_type elem,
                         Container c, Proj p
                         ) {
                     { c.push_back(p(elem)) };
                     { row.size() };
                 }
        explicit grid(R&& r, Proj p) {
            for(const auto& row : r) {
                width = row.size();
                std::transform(row.begin(), row.end(), std::back_inserter(data), p);
            }
        };

        template <std::ranges::range R>
        requires std::ranges::range<typename R::value_type>
                 && requires(
                         typename std::remove_reference_t<R>::value_type row,
                         typename std::remove_reference_t<R>::value_type::value_type elem,
                         Container c
                         ) {
                     { c.push_back(elem) };
                     { row.size() };
                 }
        explicit grid(R&& r) : grid(std::forward(r), std::identity()) {};

        template <
                std::ranges::range R,
                std::invocable<typename std::remove_reference_t<R>::value_type::value_type> Proj
                >
        requires std::ranges::range<typename std::remove_reference_t<R>::value_type> &&
                std::is_aggregate_v<Container>
        explicit grid(R&& r, Proj p) {
            for(auto& row : r) {
                width = row.size();
                std::transform(row.begin(), row.end(), data.begin(), p);
            }
        };

        template <std::ranges::range R>
        requires std::ranges::range<typename std::remove_reference_t<R>::value_type> &&
                std::is_aggregate_v<Container>
        explicit grid(R&& r) : grid(std::forward(r), std::identity()) {};

        grid() : data(), width(1) {};

        void set_width(int new_width) {
            assert(data.size() % new_width == 0);
            width = new_width;
        }

        [[nodiscard]] std::size_t get_height() const {
            return data.size() / width;
        }

        auto get_size() {
            return data.size();
        }

        std::pair<int, int> get_dimensions() {
            return {width, data.size() / width};
        }

        const T& get(int i, int j) const {
            return data.at(j * width + i);
        }
        T& get(int i, int j) {
            return data.at(j * width + i);
        }

        template <std::invocable<T&> I, std::invocable<> O>
        void leveled_foreach(I inner, O outer) {
            for (auto row : *this) {
                for (const T& item : std::ranges::subrange(row.first, row.second)) {
                    inner(item);
                }
                outer();
            }
        }

        template <std::invocable<raw_iterator &> I, std::invocable<> O>
        void leveled_iterators(I inner, O outer) {
            for (auto row : *this) {
                for (auto [start, end] = row; start != end; ++start) {
                    inner(start);
                }
                outer();
            }
        }

        const Container& get_raw() const {
            return data;
        }

        row_iterator begin() const {
            return row_iterator(width, data.begin());
        }
        row_iterator end() const {
            return row_iterator(width, data.end());
        }

        [[nodiscard]] std::pair<int, int> coord_from_index(int index) const {
            return {index % width, index/width};
        }

        [[nodiscard]] std::pair<int, int> coord_from_index(const_raw_iterator index) const {
            int i = index - data.begin();
            return coord_from_index(i);
        }

        [[nodiscard]] std::optional<raw_iterator> up(std::optional<raw_iterator> curr) const {
            if (!curr || (*curr - width) < data.begin())
                return std::nullopt;
            return *curr - width;
        }
        [[nodiscard]] std::optional<raw_iterator> left(std::optional<raw_iterator> curr) const {
            if (!curr || (*curr - data.begin()) % width == 0)
                return std::nullopt;
            return *curr - 1;
        }
        [[nodiscard]] std::optional<raw_iterator> right(std::optional<raw_iterator> curr) const {
            if (!curr || (*curr - data.begin() + 1) % width == 0)
                return std::nullopt;
            return *curr + 1;
        }
        [[nodiscard]] std::optional<raw_iterator> down(std::optional<raw_iterator> curr) const {
            if (!curr || (*curr + width) >= data.end())
                return std::nullopt;
            return *curr + width;
        }

        [[nodiscard]] std::optional<const_raw_iterator> up(std::optional<const_raw_iterator> curr) const {
            if (!curr || (*curr - width) < data.begin())
                return std::nullopt;
            return *curr - width;
        }
        [[nodiscard]] std::optional<const_raw_iterator> left(std::optional<const_raw_iterator> curr) const {
            if (!curr || (*curr - data.begin()) % width == 0)
                return std::nullopt;
            return *curr - 1;
        }
        [[nodiscard]] std::optional<const_raw_iterator> right(std::optional<const_raw_iterator> curr) const {
            if (!curr || (*curr - data.begin() + 1) % width == 0)
                return std::nullopt;
            return *curr + 1;
        }
        [[nodiscard]] std::optional<const_raw_iterator> down(std::optional<const_raw_iterator> curr) const {
            if (!curr || (*curr + width) >= data.end())
                return std::nullopt;
            return *curr + width;
        }

        auto neighbour_range(const_raw_iterator current) {
            std::array neighbours{up(current), up(left(current)), left(current), down(left(current)),
                                  down(current), down(right(current)), right(current), up(right(current))};
            return neighbours;
        }
        auto cardinal_neighbour_range(const_raw_iterator current) {
            std::array neighbours{up(current), left(current), down(current), right(current)};
            return neighbours;
        }

        auto neighbour_range(raw_iterator current) {
            std::array neighbours{
                up(std::optional(current)),
                up(left(std::optional(current))),
                left(std::optional(current)),
                down(left(std::optional(current))),
                down(std::optional(current)),
                down(right(std::optional(current))),
                right(std::optional(current)),
                up(right(std::optional(current)))
            };
            return neighbours;
        }
        auto cardinal_neighbour_range(raw_iterator current) {
            std::array neighbours{
                up(std::optional(current)),
                left(std::optional(current)),
                down(std::optional(current)),
                right(std::optional(current))
            };
            return neighbours;
        }

        static auto valid_index() {
            return std::views::filter([](std::optional<raw_iterator> x) { return x.has_value(); } )
                   | std::views::transform([] (std::optional<raw_iterator> x) { return *x; });
        }

        static auto const_valid_index() {
            return std::views::filter([](std::optional<const_raw_iterator> x) { return x.has_value(); } )
                   | std::views::transform([] (std::optional<const_raw_iterator> x) { return *x; });
        }
    };

    template <typename T, typename Container>
    class grid<T, Container>::row_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = std::pair<typename Container::const_iterator, typename Container::const_iterator>;
        using difference_type = int;
        using pointer = value_type*;
        using referece = value_type&;
    private:
        typename Container::const_iterator curr;
        int width;
        value_type current_data;
    public:

        row_iterator(int _width, typename Container::const_iterator _curr) :
            width(_width),
            curr(_curr),
            current_data{curr, curr + width} {}

        void update_data() {
            current_data = {curr, curr + width};
        }

        value_type operator*() {
            return current_data;
        }
        pointer operator->() {
            return &current_data;
        }
        row_iterator& operator++() {
            curr += width;
            update_data();
            return *this;
        }
        row_iterator operator++(int) {
            row_iterator cpy(*this);
            ++(*this);
            return cpy;
        }
        row_iterator operator+(int i) {
            row_iterator cpy(*this);
            cpy.curr += width * i;
            cpy.update_data();
            return cpy;
        }
        row_iterator& operator+=(int i) {
            curr += width * i;
            update_data();
            return *this;
        }
        row_iterator& operator--() {
            curr -= width;
            update_data();
            return *this;
        }
        row_iterator operator--(int) {
            row_iterator cpy(*this);
            --(*this);
            return cpy;
        }
        row_iterator operator-(int i) {
            row_iterator cpy(*this);
            cpy.curr -= width * i;
            cpy.update_data();
            return cpy;
        }
        int operator-(const row_iterator& other) {
            return (curr - other.curr) / width;
        }
        row_iterator& operator-=(int i) {
            curr -= width * i;
            update_data();
            return *this;
        }
        row_iterator operator[](int i) {
            return *this + i;
        }
        template <std::ranges::range R>
        row_iterator& operator=(const R& r) {
            std::copy(r, *this);
            return *this;
        }
        auto operator<=>(const row_iterator& other) const {
            return curr <=> other.curr;
        };
        bool operator==(const row_iterator& other) const {
            return curr == other.curr;
        }
        bool operator!=(const row_iterator& other) const {
            return curr != other.curr;
        }
    };
}

#endif //OX_LIB__2D_GRID_H
