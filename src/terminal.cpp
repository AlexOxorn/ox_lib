#include <ox/terminal.h>

namespace ox {
    terminal_size get_terminal_size() {
        #ifdef TIOCGSIZE
            struct ttysize ts;
            ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
            return {ts.ts_cols, ts.ts_rows};
        #elif defined(TIOCGWINSZ)
            struct winsize ts;
            ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
            return {ts.ws_col, ts.ws_row};
        #endif /* TIOCGSIZE */
        return {-1, -1};
    }
}
