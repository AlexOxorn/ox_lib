#ifndef _OXORN_TERMINAL_H
#define _OXORN_TERMINAL_H

#include <sys/ioctl.h>
#include <cstdio>
#include <unistd.h>
#include <utility>

namespace ox {
    struct terminal_size {
        int columns;
        int rows;
    };
    terminal_size get_terminal_size();
}

#endif
