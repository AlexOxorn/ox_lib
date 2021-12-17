#ifndef OXLIB_DEBUG_H
#define OXLIB_DEBUG_H

#include <typeinfo>
#include <string>
#include <string.h>

namespace ox {
    char const * errnoname(int errno_);

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

#endif //OXLIB_DEBUG_H