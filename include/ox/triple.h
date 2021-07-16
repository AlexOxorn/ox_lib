#ifndef _OXORN_TRIPLE_H
#define _OXORN_TRIPLE_H

#include <stdint.h>
#include <type_traits>
#include <concepts>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

namespace ox {
    template <typename T>
    struct triple {
        union {
            struct {
                T x, y, z;
            };
            T data[3];
        };
    };
}


#endif
