#ifndef OXLIB_IO_UTILS_H
#define OXLIB_IO_UTILS_H

#include <cstdio>
#include <unistd.h>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace ox {
    const std::filesystem::path& executable_path();
    const std::filesystem::path& executable_folder();
}

#endif // OXLIB_IO_UTILS_H