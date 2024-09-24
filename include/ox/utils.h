#ifndef OXLIB_UTILS_H
#define OXLIB_UTILS_H

#include "utils/_triple.h"
#include "utils/_bitset_container.h"
#include "utils/_concepts.h"

namespace ox {
    template <class... Ts>
    struct overload : Ts... {
        using Ts::operator()...;
    };
    template <class... Ts>
    overload(Ts...) -> overload<Ts...>;
}

#endif //OXLIB_UTILS_H
