

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
} // namespace ox

#endif // OXLIB_ARRAY_UTILS_H
