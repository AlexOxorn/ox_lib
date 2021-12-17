//
// Created by alexoxorn on 2021-12-16.
//

#ifndef OXLIB__ENDIABLE_H
#define OXLIB__ENDIABLE_H

#include <type_traits>

namespace ox {
    template<typename T>
    struct is_scalar_endianable : std::integral_constant<
            bool,
            std::is_arithmetic_v<T> ||
            std::is_pointer_v<T> ||
            std::is_enum_v<T>
    > {
    };

    template<typename T>
    class is_custom_endianable {
        typedef char small;
        struct big {
            char x[2];
        };

        template<typename C>
        static small test(decltype(&C::endian_swap)) { return '\0'; };

        template<typename C>
        static big test(...) { return "a"; };

    public:
        enum {
            value = sizeof(test<T>(0)) == sizeof(char)
        };
    };

    template<typename T>
    struct is_endianable : std::integral_constant<
            bool,
            is_scalar_endianable<T>::value ||
            is_custom_endianable<T>::value
    > {
    };

    template<typename T>
    inline constexpr bool is_scalar_endianable_v = is_scalar_endianable<T>::value;

    template<typename T>
    inline constexpr bool is_custom_endianable_v = is_custom_endianable<T>::value;

    template<typename T>
    inline constexpr bool is_endianable_v = is_endianable<T>::value;

    template<typename T>
    concept scalar_endianable = is_scalar_endianable_v<T>;
    template<typename T>
    concept custom_endianable = is_custom_endianable_v<T>;
    template<typename T>
    concept endianable = is_endianable_v<T>;
}

#endif //OXLIB__ENDIABLE_H
