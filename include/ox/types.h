#ifndef _OXORN_TYPES_H
#define _OXORN_TYPES_H

#include <stdint.h>
#include <type_traits>
#include <concepts>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

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

        ptr<T>& operator++() {
            addr += sizeof(T);
            return *this;
        }

        ptr<T> operator+(int a) {
            return ptr<T>{addr + a};
        }

        ptr<T> operator-(int a) {
            return ptr<T>{addr - a};
        }

        ptr<T>& operator+=(int a) {
            addr += a * sizeof(T);
            return *this;
        }

        ptr<T>& operator-=(int a) {
            addr -= a * sizeof(T);
            return *this;
        }

        T& operator[](int a) {
            return *(T*)(this->addr + sizeof(T) * a);
        }

        operator T*() {
            return (T*)addr;
        }

        operator base_type() {
            reutrn addr;
        }
    };

    template <>
    class ptr<void> {
        unsigned addr;
    public:
        ptr(unsigned pAddr) :addr{pAddr} {};
        ptr() = default;
    };
}


#endif
