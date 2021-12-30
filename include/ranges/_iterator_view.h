//
// Created by alexoxorn on 2021-12-16.
//

#ifndef OX_LIBVIEWS_H
#define OX_LIBVIEWS_H

#include <ranges>
#include <concepts>

namespace ox::ranges {
    template<std::ranges::input_range R>
    requires std::ranges::view<R>
    class iterators_view : public std::ranges::view_interface<iterators_view<R>> {
    public:
        class iterator {
        public:
            using base_itr = std::ranges::iterator_t<R>;
            using value_type = base_itr;
            using difference_type = std::ranges::range_difference_t<R>;

        private:
            base_itr current;

        public:
            iterator() = default;
            iterator(base_itr itr)
                : current(itr){};

            const base_itr& operator*() const {
                return this->current;
            }

            iterator& operator++() {
                current++;
                return *this;
            }

            void operator++(int) { ++current; }

            iterator operator++(int) requires std::ranges::forward_range<R> {
                auto temp = *this;
                ++*this;
                return temp;
            }

            iterator& operator--() requires std::ranges::bidirectional_range<R> {
                --current;
                return *this;
            }

            iterator operator--(int) requires std::ranges::bidirectional_range<R> {
                auto temp = *this;
                --*this;
                return temp;
            }

            iterator& operator+=(difference_type n) requires std::ranges::random_access_range<R> {
                current += n;
                return *this;
            }

            iterator& operator-=(difference_type n) requires std::ranges::random_access_range<R> {
                current -= n;
                return *this;
            }

            auto operator[](difference_type n) const requires std::ranges::random_access_range<R> { return current[n]; }

            friend bool operator==(const iterator& x, const iterator& y) requires std::equality_comparable<base_itr> {
                return x.current == y.current;
            }

            friend auto operator<=>(const iterator& x, const iterator& y) requires std::ranges::random_access_range<R> && std::three_way_comparable<base_itr> {
                return x.current <=> y.current;
            }

            friend iterator operator+(iterator i, difference_type n) requires std::ranges::random_access_range<R> {
                return {i.current + n};
            }

            friend iterator operator+(difference_type n, iterator i) requires std::ranges::random_access_range<R> {
                return {i.current + n};
            }

            friend constexpr iterator operator-(iterator i, difference_type n) requires std::ranges::random_access_range<R> {
                return {i.current - n};
            }
        };

    private:
        R range{};
        iterator _iter{std::begin(range)};

    public:
        iterators_view() = default;
        ;

        explicit iterators_view(R base)
            : range(base),
              _iter(std::begin(range)){};

        constexpr R base() const& { return range; }

        constexpr R base() && { return std::move(range); }

        constexpr iterator begin() const { return _iter; }

        constexpr iterator end() const { return std::end(range); }
    };

    template<class R>
    iterators_view(R&& base) -> iterators_view<std::ranges::views::all_t<R>>;

    namespace details {
        struct iterators_range_adaptor {
            constexpr iterators_range_adaptor() = default;

            template<std::ranges::viewable_range R>
            constexpr auto operator()(R&& r) const {
                return iterators_view(std::forward<R>(r));
            }
        };

        template<std::ranges::viewable_range R>
        constexpr auto operator|(R&& r, iterators_range_adaptor const& a) {
            return a(std::forward<R>(r));
        }
    } // namespace details

    namespace views {
        inline details::iterators_range_adaptor iterators;
    }
} // namespace ox::ranges

#endif // OX_LIBVIEWS_H
