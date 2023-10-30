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
#include <ranges>
#include <functional>
#include "ox/ranges.h"
#include "ox/algorithms.h"

namespace ox {
    template<typename T, typename Container = std::vector<T>>
    class grid {
    protected:
        Container data;
        int width;

        void constexpr check_bound(int i, int j) const {
            if (i < 0 || i >= width)
                throw std::out_of_range("Width index out of range");
            if (j < 0 || size_type(j) >= get_height())
                throw std::out_of_range("Height index out of range");
        }

        [[nodiscard]] bool constexpr inbounds(int i, int j) const {
            if (i < 0 || i >= width)
                return false;
            if (j < 0 || size_type(j) >= get_height())
                return false;
            return true;
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

        template<typename... I>
        requires std::constructible_from<Container, I...>
        constexpr explicit grid(int _width, I... args)
            : data(args...),
              width(_width){};

        template<typename... I>
        constexpr explicit grid(int _width, I... args)
            : data({args...}),
              width(_width){};

        template<std::ranges::range R>
        requires std::constructible_from<Container, decltype(std::ranges::begin(std::declval<R>())), decltype(std::ranges::end(std::declval<R>()))>
        constexpr explicit grid(int _width, R&& r)
            : data(std::ranges::begin(r), std::ranges::end(r)),
              width(_width){};

        constexpr explicit grid(int _width, const std::initializer_list<T>& r) requires std::constructible_from<Container, std::initializer_list<T>> : data(r)
            , width(_width){};

        constexpr explicit grid(int _width, const std::initializer_list<T>& r) : grid(_width, std::views::all(r)) {};

        template<std::ranges::range R>
        constexpr explicit grid(int _width, R&& r)
            : width(_width) {
            if constexpr (std::ranges::random_access_range<R>) {
                data.reserve(std::ranges::distance(r));
            }
            std::copy(std::ranges::begin(r), std::ranges::end(r), std::back_inserter(data));
        };

        template<std::ranges::range R, std::invocable<typename std::remove_reference_t<R>::value_type::value_type> Proj>
        requires std::ranges::range<typename std::remove_reference_t<R>::value_type> && requires(
               typename std::remove_reference_t<R>::value_type row,
               typename std::remove_reference_t<R>::value_type::value_type elem,
               Container c, Proj p) {
            {c.push_back(std::invoke(p, elem))};
            {row.size()};
        }
        constexpr explicit grid(R&& r, Proj p) {
            for (const auto& row : r) {
                width = row.size();
                std::transform(row.begin(), row.end(), std::back_inserter(data), p);
            }
        };

        template<std::ranges::range R>
        requires std::ranges::range<typename R::value_type> && requires(
               typename std::remove_reference_t<R>::value_type row,
               typename std::remove_reference_t<R>::value_type::value_type elem,
               Container c) {
            {c.push_back(elem)};
            {row.size()};
        }
        constexpr explicit grid(R&& r)
            : grid(std::forward<R>(r), std::identity()){};

        template<
               std::ranges::range R,
               std::invocable<typename std::remove_reference_t<R>::value_type::value_type> Proj>
        requires std::ranges::range<typename std::remove_reference_t<R>::value_type> && std::is_aggregate_v<Container>
        constexpr explicit grid(R&& r, Proj p) {
            for (auto& row : r) {
                width = row.size();
                std::transform(row.begin(), row.end(), data.begin(), p);
            }
        };

        template<std::ranges::range R>
        requires std::ranges::range<typename std::remove_reference_t<R>::value_type> && std::is_aggregate_v<Container>
        constexpr explicit grid(R&& r)
            : grid(std::forward(r), std::identity()){};

        constexpr grid()
            : data(),
              width(1){};

        constexpr void set_width(int new_width) {
            assert(data.size() % new_width == 0);
            width = new_width;
        }

        [[nodiscard]] constexpr size_type get_height() const {
            return data.size() / width;
        }

        [[nodiscard]] constexpr size_type get_width() const {
            return width;
        }

        constexpr auto get_size() const {
            return data.size();
        }

        [[nodiscard]] constexpr std::pair<size_type, size_type> get_dimensions() const {
            return {width, data.size() / width};
        }

        constexpr typename Container::const_reference at(size_type i, size_type j) const {
            check_bound(i, j);
            return data.at(j * width + i);
        }
        constexpr typename Container::reference at(size_type i, size_type j) {
            check_bound(i, j);
            return data.at(j * width + i);
        }

        constexpr typename std::optional<typename Container::value_type> get(size_type i, size_type j) const {
            if (!inbounds(i, j))
                return std::nullopt;
            return data[j * width + i];
        }
        constexpr typename std::optional<typename Container::value_type> get(size_type i, size_type j) {
            if (!inbounds(i, j))
                return std::nullopt;
            return data[j * width + i];
        }

        constexpr typename Container::const_reference operator[](int i) {
            return data[i];
        }
        constexpr typename Container::reference operator[](int i) const {
            return data[i];
        }

        constexpr auto operator<=>(const grid& other)  const  requires std::three_way_comparable<Container> {
            return data <=> other.data;
        }
        constexpr bool operator==(const grid& other)  const  requires std::equality_comparable<Container> {
            return data == other.data;
        }

        #ifndef _LIBCPP_VERSION
        constexpr auto operator<=>(const grid& other) const requires (!std::three_way_comparable<Container> && std::three_way_comparable<T>) {
            return std::lexicographical_compare_three_way(data.begin(), data.end(), other.data.begin(), other.data.end());
        }
        #else
        constexpr auto operator<(const grid& other) const requires (std::three_way_comparable<T>) {
            return std::lexicographical_compare(data.begin(), data.end(), other.data.begin(), other.data.end());
        }
        constexpr auto operator<=(const grid& other) const requires (std::three_way_comparable<T>) {
            return *this < other || *this == other;
        }
        constexpr auto operator>=(const grid& other) const requires (std::three_way_comparable<T>) {
            return !(*this < other);
        }
        constexpr auto operator>(const grid& other) const requires (std::three_way_comparable<T>) {
            return *this != other && *this >= other;
        }
        #endif
        constexpr bool operator==(const grid& other) const requires (!std::equality_comparable<Container> &&
                std::equality_comparable<T>) {
            auto [it1, it2] = std::mismatch(data.begin(), data.end(), other.begin(), other.end());
            return it1 == data.end() && it2 == data.end();
        }

        template<std::invocable<T&> I, std::invocable<> O>
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

        template<std::invocable<raw_iterator&> I, std::invocable<> O>
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

        constexpr const Container& get_raw() const {
            return data;
        }

        Container& get_raw() {
            return data;
        }

        const_iterator begin() const {
            return const_iterator(width, data.begin());
        }
        const_iterator end() const {
            return const_iterator(width, data.end());
        }

        iterator begin() {
            return iterator(width, data.begin());
        }
        iterator end() {
            return iterator(width, data.end());
        }

        [[nodiscard]] std::pair<int, int> coord_from_index(int index) const {
            return {index % width, index / width};
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
                   up(right(std::optional(current)))};
            return neighbours;
        }
        auto cardinal_neighbour_range(raw_iterator current) {
            std::array neighbours{
                   up(std::optional(current)),
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

    template<typename T, typename Container>
    template<typename BaseIterator>
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
        row_iterator(): width{0}, current_data() {};

        row_iterator(int _width, BaseIterator _curr)
            : width(_width),
              current_data{_curr, _curr + width} {}


        const_reference operator*() const {
            return current_data;
        }
        const_pointer operator->() const {
            return &current_data;
        }
        row_iterator& operator++() {
            current_data = value_type{
                    current_data.end(),
                    current_data.end() + width
            };
            return *this;
        }
        row_iterator operator++(int) {
            row_iterator cpy(*this);
            ++(*this);
            return cpy;
        }
        row_iterator operator+(int i) const {
            row_iterator cpy(*this);
            cpy.current_data = value_type{
                    current_data.begin() + width * i,
                    current_data.end() + width * i
            };
            return cpy;
        }
        row_iterator& operator+=(int i) {
            current_data = value_type{
                    current_data.begin() + width * i,
                    current_data.end() + width * i
            };
            return *this;
        }
        row_iterator& operator--() const {
            current_data = value_type{
                    current_data.begin() - width,
                    current_data.begin()
            };
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
            cpy.current_data = value_type{
                    current_data.begin() - width * i,
                    current_data.end() - width * i
            };
            return cpy;
        }
        int operator-(const row_iterator& other) const {
            return (current_data.begin() - other.current_data.begin()) / width;
        }
        row_iterator& operator-=(int i) {
            current_data = value_type{
                    current_data.begin() - width * i,
                    current_data.end() - width * i
            };
            return *this;
        }
        row_iterator operator[](int i) const {
            return *this + i;
        }
        auto operator<=>(const row_iterator& other) const {
            return current_data.begin() <=> other.current_data.begin();
        };
        bool operator==(const row_iterator& other) const {
            return current_data.begin() == other.current_data.begin();
        }
        bool operator!=(const row_iterator& other) const {
            return current_data.begin() != other.current_data.begin();
        }
    };
} // namespace ox

#endif // OX_LIB__2D_GRID_H
