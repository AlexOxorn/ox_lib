#ifndef OXLIB_STD_ABRIVIATIONS_H
#define OXLIB_STD_ABRIVIATIONS_H

#include <ranges>
#include <chrono>
#include <filesystem>

namespace ox::std_abbreviations {
    namespace stdr = std::ranges;
    namespace stdv = std::views;
    namespace stdch = std::chrono;
    namespace stdfs = std::filesystem;

    template <typename iter>
    using iter_v_type = typename std::iterator_traits<iter>::value_type;
}

#endif //OXLIB_STD_ABRIVIATIONS_H
