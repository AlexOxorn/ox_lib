#ifndef OXLIB__TRIPLE_H
#define OXLIB__TRIPLE_H

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

#endif //OXLIB__TRIPLE_H
