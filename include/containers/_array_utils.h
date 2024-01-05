

#ifndef OXLIB_ARRAY_UTILS_H
#define OXLIB_ARRAY_UTILS_H

namespace ox {
    template <typename T, typename... Args>
    requires(std::is_convertible_v<Args, T> && ...)
    auto pack_array(T first, Args... rest) {
        return std::array<T, sizeof...(Args) + 1>{first, static_cast<T>(rest)...};
    }

    template <typename Func, typename T, std::size_t N, std::size_t... Is>
    auto unpack_array(Func&& f, const std::array<T, N>& arr, std::index_sequence<Is...>) {
        return f(arr[Is]...);
    }

    template <typename Func, typename T, std::size_t N>
    auto unpack_array(Func&& f, const std::array<T, N>& arr) {
        return unpack_array(std::forward<Func>(f), arr, std::make_index_sequence<N>());
    }

    template <typename Tuple, std::size_t... Is>
    auto array_from_tuple(const Tuple& t, std::index_sequence<Is...>) {
        return std::array<decltype(auto(std::get<0>(t))), sizeof...(Is)>{std::get<Is>(t)...};
    }

    template <typename... T>
    auto array_from_tuple(const std::tuple<T...>& t) {
        return array_from_tuple(t, std::index_sequence<sizeof...(T)>());
    }

    template <typename... T>
    auto array_from_tuple(const std::pair<T...>& t) {
        return array_from_tuple(t, std::make_index_sequence<2>());
    }

    template <typename F, std::size_t... Is>
    constexpr auto map_index_sequence(std::index_sequence<Is...>, F f) {
        return std::index_sequence<f(Is)...>();
    }

#define transform_N_template template <size_t Elm, template <typename...> typename T, typename... Types, typename Func, std::size_t... Pre, std::size_t... Post>
#define transform_N_args const T<Types...>& x, const Func& f, std::index_sequence<Pre...>, std::index_sequence<Post...>

    transform_N_template
    auto transform_N(transform_N_args) {
        return T{std::get<Pre>(x)..., f(std::get<Elm>(x)), std::get<Post>(x)...};
    }

    transform_N_template
    requires (sizeof...(Pre) == 0) && (sizeof...(Post) != 0)
    auto transform_N(transform_N_args) {
        return T{f(std::get<Elm>(x)), std::get<Post>(x)...};
    }

    transform_N_template
    requires (sizeof...(Pre) != 0) && (sizeof...(Post) == 0)
    auto transform_N(transform_N_args) {
        return T{std::get<Pre>(x)..., f(std::get<Elm>(x))};
    }

    transform_N_template
    requires (sizeof...(Pre) == 0) && (sizeof...(Post) == 0)
    auto transform_N(transform_N_args) {
        return T{f(std::get<Elm>(x))};
    }

    template <size_t Elm, template <typename...> typename T, typename Func, typename... Types>
    auto transform_N(const T<Types...>& x, const Func& f) {
        return transform_N<Elm>(x,
                                f,
                                std::make_index_sequence<Elm>(),
                                map_index_sequence(std::make_index_sequence<sizeof...(Types) - Elm - 1>(),
                                                   [](size_t s) { return s + Elm + 1; }));
    }
    template <size_t Elm, typename Func>
    auto transform_N(const Func& f) {
        return [&f] <template <typename...> typename T, typename... Types> (const T<Types...>& x) {
            return transform_N<Elm>(x, f);
        };
    }
} // namespace ox

#endif // OXLIB_ARRAY_UTILS_H
