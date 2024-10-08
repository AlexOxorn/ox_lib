//
// Created by alexoxorn on 25/09/24.
//

#ifndef OXLIB_UTILS_H
#define OXLIB_UTILS_H

#include <concepts>

namespace ox {
#include <tuple>

    template <typename T, typename Enabled = void>
    struct is_tuple_like : std::false_type {};

    template <typename T>
    struct is_tuple_like<T, typename std::enable_if<(std::tuple_size<T>::value > 0)>::type> : std::true_type {};

    template <typename T>
    concept tuple_like = is_tuple_like<T>::value;

    template <typename Func>
    struct apply_bind {
        Func f;

        explicit apply_bind(Func f) : f(std::move(f)) {};

        template <tuple_like T>
        auto operator()(T t)
        requires requires { std::apply(f, t); }
        {
            return std::apply(f, t);
        }

        template <tuple_like T>
        auto operator()()
        requires requires { f(); }
        {
            return std::apply(f);
        }
    };
} // namespace ox

#endif // OXLIB_UTILS_H
