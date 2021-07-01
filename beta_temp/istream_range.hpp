#ifndef _OXORN_ISTREAM_RANGE_H
#define _OXORN_ISTREAM_RANGE_H

#include <iterator>
#include <iostream>
#include <fstream>
#include <string>

namespace ox {
    class istream_range {
        std::istream& stream;
        char seperator;
        class iterator {
            const istream_range* range = nullptr;
            std::string line;
            int count = 0;
            bool end = false;
        public:
            iterator(const istream_range& parent);
            iterator(bool e) : end{e} {};
            bool operator==(const iterator& other);
            std::string operator*();
            iterator& operator++();
        };
        static const iterator ending;
    public:
        istream_range(std::istream& i, char sep = '\n');
        iterator begin();
        iterator end();
    };
}

#endif
