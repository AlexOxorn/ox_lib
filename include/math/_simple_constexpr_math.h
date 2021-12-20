//
// Created by alexoxorn on 2021-12-16.
//

#ifndef OX_LIB__SIMPLE_CONSTEXPR_MATH_H
#define OX_LIB__SIMPLE_CONSTEXPR_MATH_H

#include <numeric>
#include <cassert>

namespace ox {
    constexpr int power_of_2(unsigned x) {
        if (x == 0) return 1;
        int to_return = 1;
        for(int i = 0; i < x; i++) {
            to_return <<= 1;
        }
        return to_return;
    }

    constexpr int triangle_sum(int x) {
        assert(x >= 0);
        auto y = stdv::iota(1, x+1);
        return std::accumulate(y.begin(), y.end(), 0);
    }
}

#endif //OX_LIB__SIMPLE_CONSTEXPR_MATH_H
