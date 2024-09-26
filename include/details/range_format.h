//
// Created by alexoxorn on 9/23/24.
//

#ifndef ADVENTOFCODE_RANGE_FORMAT_H
#define ADVENTOFCODE_RANGE_FORMAT_H

#ifndef __cpp_lib_format_ranges
#include <format>
template <typename V, char open, char close>
struct range_formatter {
    std::formatter<V> underlying;

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        // ensure that the format specifier is empty
        if (ctx.begin() != ctx.end() && *ctx.begin() != '}') {
            throw std::format_error("invalid format");
        }

        // ensure that the underlying type can parse an empty specifier
        auto out = underlying.parse(ctx);

        // conditionally format as debug, if the type supports it
        if constexpr (requires { underlying.set_debug_format(); }) {
            underlying.set_debug_format();
        }
        return out;
    }

    template <typename R, typename FormatContext>
    requires std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<R>>, V>
    constexpr auto format(R&& r, FormatContext& ctx) const {
        auto out = ctx.out();
        *out++ = open;
        auto first = std::ranges::begin(r);
        auto last = std::ranges::end(r);
        if (first != last) {
            // have to format every element via the underlying formatter
            ctx.advance_to(std::move(out));
            out = underlying.format(*first, ctx);
            for (++first; first != last; ++first) {
                *out++ = ',';
                *out++ = ' ';
                ctx.advance_to(std::move(out));
                out = underlying.format(*first, ctx);
            }
        }
        *out++ = close;
        return out;
    }
};
#endif

#endif // ADVENTOFCODE_RANGE_FORMAT_H
