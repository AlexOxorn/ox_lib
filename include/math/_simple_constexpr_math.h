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
        auto y = std::views::iota(1, x+1);
        return std::accumulate(y.begin(), y.end(), 0);
    }

    constexpr inline int numberOfDigits(int x, int base = 10) {
        int count = 1;
        while (x /= base) { count++; }
        return count;
    }

    template <std::integral L, std::unsigned_integral R>
    constexpr auto fast_pow(L x, R p) -> decltype(x) {
        if (p == 0)
            return 1;
        if (p == 1)
            return x;

        auto tmp = fast_pow(x, p / 2);
        if (p % 2 == 0)
            return tmp * tmp;
        else
            return x * tmp * tmp;
    }
}

#endif //OX_LIB__SIMPLE_CONSTEXPR_MATH_H
