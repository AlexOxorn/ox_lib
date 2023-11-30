#ifndef OXLIB__STRING_ALGORITHMS_H
#define OXLIB__STRING_ALGORITHMS_H

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
    template<
            std::forward_iterator iter,
            typename string_type,
            typename return_type = iter_v_type<iter>,
            typename unary_transform = std::identity
    >
    requires requires(return_type out, string_type sep, iter_v_type<iter> in, unary_transform op) {
        { out.append(op(in)) };
        { out.append(sep) };
    }
    constexpr return_type &join(
            iter begin,
            iter end,
            string_type separator,
            return_type &result,
            unary_transform op = unary_transform{}
    ) {
        if (begin == end)
            return result;
        result.append(op(*begin));
        ++begin;
        for (; begin != end; ++begin) {
            result.append(separator);
            result.append(op(*begin));
        }
        return result;
    }

}

#endif //OXLIB__STRING_ALGORITHMS_H
