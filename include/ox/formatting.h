#ifndef OXLIB_FORMATING_H
#define OXLIB_FORMATING_H

#include <initializer_list>
#include <string>
#include <ox/algorithms.h>

namespace ox {
    namespace escape {
        enum Cursor : char {
            move = 'H',
            move_up = 'A',
            move_down = 'B',
            move_right = 'C',
            move_left = 'D',
            next_line_begin_down = 'E',
            previous_line_begin_down = 'F',
            move_to_column = 'G',
            save = 's',
            restore = 'u',
        };

        enum Clear : char {
            clear_screen = 'J',
            clear_line = 'K',
        };

        enum Scope : int {
            to_end = 0,
            to_beginning = 1,
            all = 2,
        };

        enum Format : int {
            reset = 0,
            bold = 1,
            dim = 2,
            italic = 3,
            underline = 4,
            blinking = 5,
            inverse = 7,
            invisible = 8,
            strikethrough = 9,
            black = 30,
            red = 31,
            green = 32,
            yellow = 33,
            blue = 34,
            magenta = 35,
            cyan = 36,
            white = 37,
            direct_color = 38,
            background_black = 40,
            background_red = 41,
            background_green = 42,
            background_yellow = 43,
            background_blue = 44,
            background_magenta = 45,
            background_cyan = 46,
            background_white = 47,
            background_direct = 48,
        };

        enum ColorMode : int {
            id = 5,
            rgb = 2,
        };
    }

    constexpr auto escape_opening = "\033[";

    template<char suffix, char prefix = '\0'>
    class escape_code {
        std::vector<std::string> arguments{};
        std::string sequence;
    public:
        escape_code(const std::initializer_list<int>& list) {
            sequence = std::string{escape_opening};
            if constexpr (prefix)
                sequence.push_back(prefix);

            ox::join(
                list.begin(),
                list.end(),
                ";",
                sequence,
                static_cast<std::string(*)(int)>(std::to_string)
            );

            sequence.push_back(suffix);
        };

        operator const char*() const {
            return sequence.c_str();
        }

        operator const std::string&() const {
            return sequence;
        }

        [[nodiscard]] const char* c_str() const {
            return sequence.c_str();
        }
    };

    using format = escape_code<'m'>;

    using clear_screen = escape_code<'J'>;
    using clear_line = escape_code<'K'>;

    using move_cursor = escape_code<'H'>;
    using move_cursor_up = escape_code<'A'>;
    using move_cursor_down = escape_code<'B'>;
    using move_cursor_right = escape_code<'C'>;
    using move_cursor_left = escape_code<'D'>;
    using move_cursor_begin_down = escape_code<'E'>;
    using move_cursor_begin_up = escape_code<'F'>;
    using move_cursor_column = escape_code<'G'>;
    using save_cursor_position = escape_code<'s'>;
    using restore_cursor_position = escape_code<'u'>;
}

#endif
