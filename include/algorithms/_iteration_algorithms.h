//
// Created by alexoxorn on 2021-12-16.
//

#ifndef OXLIB__ITERATION_ALGORITHMS_H
#define OXLIB__ITERATION_ALGORITHMS_H

#include <iterator>
#include <concepts>
#include <string>
#include <ranges>
#include <type_traits>
#include <functional>
#include <utility>
#include <ox/std_abbreviation.h>

namespace ox {
    using namespace ox::std_abbreviations;
    template<std::input_iterator in, typename out>
    requires std::output_iterator<out, iter_v_type<in>>
             || std::ranges::range<iter_v_type<in>>
    constexpr out deep_flatten(in start, in end, out d_start) {
        while (start != end) {
            if constexpr (std::output_iterator<out, iter_v_type<in>> ) {
                *d_start++ = *start++;
            } else {
                d_start = deep_flatten(start->begin(), start->end(), d_start);
                start++;
            }
        }
        return d_start;
    }

    template<std::input_iterator in, typename out>
    requires std::ranges::range<iter_v_type<in>> &&
             std::output_iterator<out, typename iter_v_type<in>::value_type>
    constexpr out flatten(in start, in end, out d_start) {
        while (start != end) {
            for(auto a : *start) {
                *d_start++ = a;
            }
            start++;
        }
        return d_start;
    }

    template<typename T, size_t N, size_t M>
    constexpr std::array<T, N * M>
    flatten(const std::array<std::array<T, N>, M>& array_of_arrays) {
        std::array<T, N * M> to_return;
        flatten(array_of_arrays.begin(), array_of_arrays.end(), to_return.begin());
        return to_return;
    }

    template<typename Map, typename Key, typename T>
    void insert_or_overwrite(Map& m, const Key& k, T&& t) {
        auto location = m.find(k);
        if (location != m.end()) {
            location->second = std::forward<T>(t);
            return;
        }
        m.insert(std::pair{k, std::forward<T>(t)});
    }
}

#endif //OXLIB__ITERATION_ALGORITHMS_H
