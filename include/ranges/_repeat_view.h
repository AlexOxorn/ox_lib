//
// Created by alexoxorn on 2021-12-16.
//

#ifndef OX_LIB_REPEAT_VIEWS_H
#define OX_LIB_REPEAT_VIEWS_H

#include <ranges>
#include <concepts>

namespace ox::ranges {
    template<std::ranges::input_range R>
    requires std::ranges::view<R>
    class repeat_view : public std::ranges::view_interface<repeat_view<R>> {
    public:
        class iterator {
        public:
            using base_itr = std::ranges::iterator_t<R>;
            using sentinel = std::ranges::sentinel_t<R>;
            using value_type = std::ranges::range_value_t<R>;
            using difference_type = std::ranges::range_difference_t<R>;

        private:
            base_itr current;
            const base_itr begin;
            const sentinel end;
            const bool ending = false;

        public:
            iterator() : current(), begin(), end(), ending(true) {};
            explicit iterator(base_itr itr, sentinel end)
                : current(itr), begin(itr), end(end){};

            value_type operator*() const {
                return *current;
            }

            iterator& operator++() {
                current++;
                if (current == end) {
                    current = begin;
                }
                return *this;
            }

            void operator++(int) { ++current; }

            iterator operator++(int) requires std::ranges::forward_range<R> {
                auto temp = *this;
                ++*this;
                return temp;
            }

            iterator& operator--() requires std::ranges::bidirectional_range<R> {
                if (current == begin) {
                    current = end;
                }
                --current;
                return *this;
            }

            iterator operator--(int) requires std::ranges::bidirectional_range<R> {
                auto temp = *this;
                --*this;
                return temp;
            }

            friend bool operator==(const iterator& x, const iterator& y) requires std::equality_comparable<base_itr> {
                return x.current == y.current;
            }
        };

    private:
        R range{};
        iterator _iter{std::ranges::begin(range), std::ranges::end(range)};

    public:
        repeat_view() = default;

        explicit repeat_view(R base)
            : range(base),
              _iter(std::ranges::begin(range), std::ranges::end(range)){};

        constexpr R base() const& { return range; }

        constexpr R base() && { return std::move(range); }

        constexpr iterator begin() const { return _iter; }

        constexpr iterator end() const { return iterator(); }
    };

    template<class R>
    repeat_view(R&& base) -> repeat_view<std::ranges::views::all_t<R>>;

    namespace details {
        struct repeat_range_adaptor_closure {
            constexpr repeat_range_adaptor_closure() = default;

            template<std::ranges::viewable_range R>
            constexpr auto operator()(R&& r) const {
                return repeat_view(std::forward<R>(r));
            }
        };

        template<std::ranges::viewable_range R>
        constexpr auto operator|(R&& r, repeat_range_adaptor_closure const& a) {
            return a(std::forward<R>(r));
        }

        struct repeat_range_adaptor {
            template<std::ranges::viewable_range R>
            constexpr auto operator()(R&& r, std::iter_difference_t<std::ranges::iterator_t<R>> count) {
                return repeat_range_adaptor(std::forward<R>(r), count);
            }

            constexpr auto operator()() {
                return repeat_range_adaptor_closure();
            }
        };
    } // namespace details

    namespace views {
        inline details::repeat_range_adaptor repeat;
    }
} // namespace ox::ranges

#endif // OX_LIB_REPEAT_VIEWS_H
