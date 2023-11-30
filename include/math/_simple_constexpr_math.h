#ifndef OX_LIB__SIMPLE_CONSTEXPR_MATH_H
#define OX_LIB__SIMPLE_CONSTEXPR_MATH_H

#include <numeric>
#include <cassert>

namespace ox {
    constexpr inline int power_of_2(unsigned x) {
        return 1 << x;
    }

    constexpr inline int triangle_sum(int x) {
        assert(x >= 0);
        auto y = stdv::iota(1, x+1);
        return std::accumulate(y.begin(), y.end(), 0);
    }

    constexpr inline int numberOfDigits(int x, int base = 10) {
        int count = 1;
        while (x /= base) { count++; }
        return count;
    }
}

#endif //OX_LIB__SIMPLE_CONSTEXPR_MATH_H
