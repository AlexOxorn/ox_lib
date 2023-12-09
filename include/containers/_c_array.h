#ifndef OXLIB__C_ARRAY_H
#define OXLIB__C_ARRAY_H

#include <iterator>
#include <array>

namespace ox {
    template <typename T, size_t N>
    constexpr size_t array_size(T (&)[N]) {
        return N;
    }

    template <typename T, size_t N>
    std::array<T, N> c_to_std_array(const T (&carr)[N]) {
        std::array<T, N> to_return{};
        std::copy(std::begin(carr), std::end(carr), to_return.begin());
        return to_return;
    }

    template <typename T, typename... Args>
    requires(std::is_convertible_v<Args, T> && ...)
    auto pack_array(T first, Args... rest) {
        return std::array<T, sizeof...(Args) + 1>{first, static_cast<T>(rest)...};
    }
}

#endif //OXLIB__C_ARRAY_H
