//
// Created by alexoxorn on 2021-12-16.
//

#ifndef OX_LIB__BITSET_CONTAINER_H
#define OX_LIB__BITSET_CONTAINER_H

#include <bitset>

namespace ox {
    template <size_t WIDTH>
    class bitset_container : public std::bitset<WIDTH> {
        class const_iterator {
            friend bitset_container;
            const bitset_container* r;
            int index;
            int dir;

            const_iterator(const bitset_container& _r, int _index, int _dir = 1) : r(&_r), index(_index), dir(_dir) {};
        public:
            const_iterator& operator++() { index += dir; return *this; }
            const_iterator& operator--() { index -= dir; return *this; }
            bool operator*() { return (*r)[index]; }
            auto operator<=>(const const_iterator& other) const = default;
        };
        class iterator {
            friend bitset_container;
            bitset_container* r;
            int index;
            int dir;

            iterator(bitset_container& _r, int _index, int _dir = 1) : r(&_r), index(_index), dir(_dir) {};
        public:
            iterator& operator++() { index += dir; return *this; }
            iterator& operator--() { index -= dir; return *this; }
            auto operator*() { return (*r)[index]; }
            auto operator<=>(const iterator& other) const = default;
        };
    public:
        [[nodiscard]] const_iterator begin() const { return {*this, 0}; }
        [[nodiscard]] const_iterator end() const { return {*this, WIDTH}; }
        [[nodiscard]] iterator begin() { return {*this, 0}; }
        [[nodiscard]] iterator end() { return {*this, WIDTH}; }
    };
}

#endif //OX_LIB__BITSET_CONTAINER_H
