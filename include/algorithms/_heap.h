//
// Created by alexoxorn on 2021-12-18.
//

#ifndef OX_LIB__HEAP_H
#define OX_LIB__HEAP_H

#include <algorithm>
#include <ranges>
#include <utility>

namespace ox {
    template <std::ranges::random_access_range Container, typename Value, typename Comp = std::ranges::less>
    requires requires (Container c, Value v) { {c.push_back(v)}; }
    void push_heap_value(Container& c, Value&& v, Comp comp = {}) {
        c.push_back(std::forward<Value>(v));
        std::ranges::push_heap(c, comp);
    }

    template <std::ranges::random_access_range Container, typename Comp = std::ranges::less>
    requires requires (Container c) { {c.pop_back()}; }
    decltype(auto) pop_heap_value(Container& c, Comp comp = {}) {
        auto temp = c.front();
        std::ranges::pop_heap(c, comp);
        c.pop_back();
        return temp;
    }
}

#endif //OX_LIB__HEAP_H
