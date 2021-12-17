#include <ox/formatting.h>
#include <cstdio>
#include <iostream>
#include <ios>

int main() {
    std::cout 
        << ox::format{ox::escape::red, ox::escape::underline}
        << "Hello "
        << ox::format{ox::escape::bold, ox::escape::blue}
        << "World "
        << ox::format{ox::escape::reset, ox::escape::italic, ox::escape::green}
        << "Alex "
        << ox::format{ox::escape::reset}
        << std::endl;
}
