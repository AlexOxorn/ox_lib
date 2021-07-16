#ifndef _OXORN_TYPES_H
#define _OXORN_TYPES_H

#include <stdint.h>
#include <type_traits>
#include <concepts>
#include <typeinfo>
#include <string>

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

    template <typename T>
    std::string type_name() {
        using TR = typename std::remove_reference<T>::type;
        std::string r = typeid(TR).name();
        if (std::is_const<TR>::value)
            r += " const";
        if (std::is_volatile<TR>::value)
            r += " volatile";
        if (std::is_lvalue_reference<T>::value)
            r += "&";
        else if (std::is_rvalue_reference<T>::value)
            r += "&&";
        return r;
    }
}


#endif
