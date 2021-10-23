//
// Created by alexoxorn on 2021-12-16.
//

#ifndef OXLIB__C_ARRAY_H
#define OXLIB__C_ARRAY_H

#include <iterator>
#include <array>

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

#endif //OXLIB__C_ARRAY_H
