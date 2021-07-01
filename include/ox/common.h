#ifndef _OXORN_COMMON_H
#define _OXORN_COMMON_H

#include <iterator>

namespace ox {
    template <typename iter>
    using iter_v_type = typename std::iterator_traits<iter>::value_type;

    template <typename T, size_t N>
    constexpr size_t array_size(T (&arr)[N]) {
      return N;
    }
}

#endif
