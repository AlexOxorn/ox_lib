//
// Created by alexoxorn on 2021-09-15.
//

#include <ox/X11Test.h>

namespace ox {
    XTextItem string_to_X(const std::string& s) {
        return XTextItem{
                const_cast<char *>(s.c_str()),
                static_cast<int>(s.length()),
                0,
                None
        };
    }
}