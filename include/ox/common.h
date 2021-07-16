#ifndef _OXORN_COMMON_H
#define _OXORN_COMMON_H

#include <iterator>
#include <array>

namespace ox {
    template <typename iter>
    using iter_v_type = typename std::iterator_traits<iter>::value_type;

    template <typename T, size_t N>
    constexpr size_t array_size(T (&)[N]) {
      return N;
    }

    template <typename T, size_t N>
    std::array<T, N> c_to_std_array(T (&carr)[N]) {
        std::array<T, N> to_return{};
        std::copy(std::begin(carr), std::end(carr), to_return.begin());
        return to_return;
    }
}

#endif
