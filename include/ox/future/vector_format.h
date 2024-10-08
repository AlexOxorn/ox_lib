//
// Created by alexoxorn on 9/23/24.
//

#ifndef OXLIB_ARRAY_FORMAT_H
#define OXLIB_ARRAY_FORMAT_H

#ifndef __cpp_lib_format_ranges
  #include "../../details/range_format.h"
  #include <vector>
template <typename T>
struct std::formatter<std::vector<T>> : range_formatter<T, '[', ']'> {};

#endif

#endif // OXLIB_ARRAY_FORMAT_H
