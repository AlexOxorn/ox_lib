//
// Created by alexoxorn on 9/23/24.
//

#ifndef ADVENTOFCODE_ARRAY_FORMAT_H
#define ADVENTOFCODE_ARRAY_FORMAT_H

#ifndef __cpp_lib_format_ranges
  #include "../../details/range_format.h"
  #include <vector>
template <typename T>
struct std::formatter<std::vector<T>> : range_formatter<T, '[', ']'> {};

#endif

#endif // ADVENTOFCODE_ARRAY_FORMAT_H
