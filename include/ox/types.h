#ifndef _OXORN_TYPES_H
#define _OXORN_TYPES_H

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
    template<typename T>
    struct is_endianable : std::integral_constant<
        bool,
        std::is_arithmetic_v<T> ||
        std::is_pointer_v<T> ||
        std::is_enum_v<T>
    > {};

    template<typename T>
    inline constexpr bool is_endianable_v = is_endianable<T>::value;

    template<typename T>
    concept endianable = is_endianable_v<T>;

    template<typename T, std::integral base_type=u32>
    class ptr {
        base_type addr;
    public:
        ptr() = default;

        T operator*() {
            return *(T*)addr;
        }

        ptr<T, base_type>& operator++() {
            addr += sizeof(T);
            return *this;
        }

        ptr<T, base_type> operator+(int a) {
            return ptr<T>{addr + a};
        }

        ptr<T, base_type> operator-(int a) {
            return ptr<T>{addr - a};
        }

        ptr<T, base_type>& operator+=(int a) {
            addr += a * sizeof(T);
            return *this;
        }

        ptr<T, base_type>& operator-=(int a) {
            addr -= a * sizeof(T);
            return *this;
        }

        T& operator[](int a) {
            return *(T*)(this->addr + sizeof(T) * a);
        }

        bool operator<=>(const ptr<T, base_type>& other) const = default;

        operator T*() {
            return (T*)addr;
        }

        explicit operator base_type() {
            return addr;
        }

        operator bool() {
            return addr != 0;
        }
    };

    template <std::integral base_type>
    class ptr<void, base_type> {
        base_type addr;
    public:
        ptr(base_type pAddr) :addr{pAddr} {};
        ptr() = default;

        operator void*() {
            return (void*)addr;
        }

        explicit operator base_type() {
            return addr;
        }

        bool operator<=>(const ptr<void, base_type>& other) const = default;

        operator bool() {
            return addr != 0;
        }
    };

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
