#ifndef OXLIB_IO_ISTREAM_CONTAINER_H
#define OXLIB_IO_ISTREAM_CONTAINER_H

#include <cstdio>
#include <unistd.h>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace ox {
    class line : public std::string {
        using std::string::string;
        friend std::istream & operator>>(std::istream& in, line& l) {
            return std::getline(in, l);
        }
    };

    template <typename T, typename InputStream>
    class istream_container : public InputStream {
    public:
        using value_type = T;
        using InputStream::InputStream;
        std::istream_iterator<T> begin() { return std::istream_iterator<T>(*this); }
        std::istream_iterator<T> end() { return {}; }
    };

#define stream_container_definer(type, namespc) \
    template <typename T> using type##_container = istream_container<T, namespc::type>

    stream_container_definer(ifstream, std);
    stream_container_definer(istringstream, std);
#undef  stream_container_definer
}

#endif //OXLIB_IO_ISTREAM_CONTAINER_H