//
// Created by alexoxorn on 2021-12-16.
//

#pragma once

#include <ranges>
#include <concepts>
#include <cstdio>
#include <type_traits>

namespace ox::ranges {
    template <std::ranges::input_range R, std::ranges::input_range S>
    class set_intersection_view : public std::ranges::view_interface<set_intersection_view<R, S>> {
    public:
        class iterator {
        public:
            using base_itr = std::ranges::iterator_t<const R>;
            using base_filter_itr = std::ranges::iterator_t<const S>;
            using value_type = std::common_type<std::ranges::range_value_t<R>, std::ranges::range_value_t<S>>::type;
            using difference_type = std::ranges::range_difference_t<R>;
        private:
            base_itr first1;
            base_itr last1;
            base_filter_itr first2;
            base_filter_itr last2;
            bool end;
        public:
            iterator() : end{true} {}
            iterator(const R& r, const S& s) :
                    first1(std::begin(r)), last1(std::end(r)), first2(std::begin(s)), last2(std::end(s)), end(false) {
                ++(*this);
            };

            const value_type operator*() const { return *(this->first1); }

            iterator& operator++() {
                while (first1 != last1 && first2 != last2) {
                    if (*first1 < *first2) {
                        ++first1;
                    } else {
                        if (!(*first2 < *first1)) {
                            ++first2;
                            return *this;
                        }
                        ++first2;
                    }
                }
                end = true;
                return *this;
            }

            iterator operator++(int)
            requires std::ranges::forward_range<R>
            {
                auto temp = *this;
                ++*this;
                return temp;
            }

            friend bool operator==(const iterator& x, const iterator& y)
            requires std::equality_comparable<base_itr> && std::equality_comparable<base_filter_itr>
            {
                if (x.end == true && y.end == true) {
                    return true;
                }
                return (x.first1 == y.first1) && (x.first2 == y.first2);
            }
        };
    private:
        R range;
        S filter;
        iterator _iter{range, filter};
    public:
        explicit set_intersection_view(R _base, S _filter) : range(_base), filter(_filter), _iter(range, filter){};

        constexpr R base() const & { return range; }

        constexpr R base() && { return std::move(range); }

        constexpr iterator begin() const { return _iter; }

        constexpr iterator end() const { return {}; }
    };

    namespace details {
        template <std::ranges::input_range S>
        struct set_intersection_view_range_adaptor {
            const S s;
            constexpr explicit set_intersection_view_range_adaptor(const S&& s) : s(s){};

            template <std::ranges::viewable_range R>
            constexpr auto operator()(const R& r) const & {
                return set_intersection_view(std::views::all(r), std::move(s));
            }

            template <std::ranges::viewable_range R>
            constexpr auto operator()(const R&& r) const & {
                return set_intersection_view(std::move(r), std::move(s));
            }
        };
        template <std::ranges::input_range S>
        struct set_intersection_view_range_adaptor<S&> {
            const S& s;
            constexpr explicit set_intersection_view_range_adaptor(const S& s) : s(s){};

            template <std::ranges::viewable_range R>
            constexpr auto operator()(const R& r) const & {
                return set_intersection_view(std::views::all(r), std::views::all(s));
            }
            template <std::ranges::viewable_range R>
            constexpr auto operator()(const R&& r) const & {
                return set_intersection_view(std::move(r), std::views::all(s));
            }
        };

        struct set_intersection_view_range_adaptor2 {
            template <class R, class S>
            [[nodiscard]] constexpr auto operator()(R&& range, S&& filter) const {
                return set_intersection_view(std::forward<R>(range), std::forward<S>(filter));
            }

            template <class S>
            requires std::constructible_from<std::decay_t<S>, S>
            [[nodiscard]] constexpr auto operator()(S&& filter) const {
                return set_intersection_view_range_adaptor<S>(std::forward<S>(filter));
            }
        };

        template <std::ranges::viewable_range R, std::ranges::viewable_range S>
        constexpr auto operator|(R&& r, const set_intersection_view_range_adaptor<S>& a) {
            return a(std::forward<R>(r));
        }
    } // namespace details

    namespace views {
        constexpr inline details::set_intersection_view_range_adaptor2 set_intersection;
    } // namespace views
} // namespace ox::ranges
