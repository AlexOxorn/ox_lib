#ifndef _OXORN_TRIPLE_H
#define _OXORN_TRIPLE_H

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <ox/types.h>
#include <ox/bytes.h>

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

    template <endianable T>
    struct triple<T> {
        union {
            struct {
                T x, y, z;
            };
            T data[3];
        };

        void endian_swap() {
            ox::bswap(data, 3);
        }
    };
}


#endif
