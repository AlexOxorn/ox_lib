#ifndef _OXORN_ALGORITHM_H
#define _OXORN_ALGORITHM_H

#include <iterator>
#include <concepts>
#include <string>
#include <ranges>
#include <type_traits>
#include <functional>
#include <ox/common.h>

namespace ox {
    template <
        std::forward_iterator iter,
        typename string_type,
        typename return_type = iter_v_type<iter>,
        typename unary_transform = std::identity
    >
    requires requires(return_type out, string_type sep, iter_v_type<iter> in, unary_transform op) {
        { out.append(op(in)) };
        { out.append(sep) };
    }
    constexpr return_type& join(
        iter begin,
        iter end,
        string_type separator,
        return_type& result,
        unary_transform op = unary_transform{}
    ) {
        if (begin == end)
            return result;
        result.append(op(*begin));
        ++begin;
        for(; begin != end; ++begin) {
            result.append(separator);
            result.append(op(*begin));
        }
        return result;
    }

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
}

#endif
