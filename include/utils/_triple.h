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
        auto begin() { return stdr::begin(data); }
        auto end() { return stdr::end(data); }
        auto begin() const { return stdr::begin(data); }
        auto end() const { return stdr::end(data); }

        triple(T x, T y, T z) : x{x}, y{y}, z{z} {};
        triple() : data{} {}
        bool operator==(const triple& other) const { return x == other.x && y == other.y && z == other.z; }
        //        bool operator<(const triple& other) const {
        //            return std::compare(data, other.data);
        //        }
        auto operator<=>(const triple& other) const {
            return std::lexicographical_compare_three_way(
                    std::begin(data), std::end(data), std::begin(other.data), std::end(other.data));
        }
    };
} // namespace ox

#endif // OXLIB__TRIPLE_H
