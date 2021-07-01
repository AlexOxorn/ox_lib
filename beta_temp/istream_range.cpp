#include "istream_range.hpp"

namespace ox {
    const istream_range::iterator istream_range::ending = istream_range::iterator{true};

    istream_range::istream_range(std::istream& i, char sep) : seperator{sep}, stream{i} {};

    istream_range::iterator istream_range::begin() {
        return iterator{*this};
    }

    istream_range::iterator istream_range::end(){
        return istream_range::ending;
    }

    istream_range::iterator::iterator(const istream_range& rng) : range{&rng} {
        if(std::getline(range->stream, line, range->seperator)) {
            count = 1;
        } else {
            end = true;
        }
    };

    bool istream_range::iterator::operator==(const iterator& other) {
        if (end && other.end || count == other.count) {
            return true;
        }
        return false;
    }

    std::string istream_range::iterator::operator*(){
        return line;
    }

    istream_range::iterator& istream_range::iterator::operator++() {
        if(std::getline(range->stream, line, range->seperator)) {
            count++;
        } else {
            end = true;
        }
        return *this;
    }
}
