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
#include <queue>
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

    template<std::random_access_iterator InputIt, typename OutputIt>
    OutputIt offset_difference(InputIt first, InputIt last, OutputIt d_first, int offset = 1)
    {
        if (last - first < offset) return d_first;

        for(first += offset; first != last; ++first, ++d_first)
            *d_first = *first - first[-offset];
        return d_first;
    }

    template<std::input_iterator InputIt, typename OutputIt>
    OutputIt offset_difference(InputIt first, InputIt last, OutputIt d_first, int offset = 1)
    {
        std::queue<typename InputIt::value_type> old_data;
        for([[maybe_unused]] int i : std::views::iota(0, offset)) {
            if (first == last)
                return d_first;
            old_data.push(*first++);
        }
        while(first != last) {
            *d_first++ = *first - old_data.front();
            old_data.pop();
            old_data.push(*first++);
        }
        return d_first;
    }

    template<std::input_iterator InputIt, std::invocable<typename std::iterator_traits<InputIt>::reference> F>
    constexpr void nested_foreach(InputIt first, InputIt last, F callable) {
        std::for_each(first, last, callable);
    }

    template<std::input_iterator InputIt, typename F, typename... Callable>
    constexpr void nested_foreach(InputIt first, InputIt last, F callable, Callable... rest) requires
            (std::invocable<F, typename std::iterator_traits<InputIt>::reference> || std::invocable<F>) &&
            std::ranges::range<typename std::iterator_traits<InputIt>::reference> &&
            (sizeof...(Callable) > 0)
    {
        for (;first != last; ++first) {
            nested_foreach(std::ranges::begin(*first), std::ranges::end(*first), rest...);
            if constexpr (std::invocable<F>)
                std::invoke(callable);
            else
                std::invoke(callable, *first);
        }
    }
}

#endif //OXLIB__ITERATION_ALGORITHMS_H
