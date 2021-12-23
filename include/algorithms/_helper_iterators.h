//
// Created by alexoxorn on 2021-12-16.
//

#ifndef OX_LIB__HELPER_ITERATORS_H
#define OX_LIB__HELPER_ITERATORS_H

#include <functional>

namespace ox {
    template<typename T>
    class predicateCounter {
        int& count;
        std::function<bool(T)> predicate;
    public:
        predicateCounter(std::function<bool(T)> pred, int& counter) : count(counter), predicate(std::move(pred)) {}

        predicateCounter& operator=(T t) {
            count += predicate(t);
            return *this;
        }

        predicateCounter& operator*() { return *this; }
        predicateCounter& operator++() { return *this;}
        predicateCounter operator++(int) { return *this; };
    };

    template <std::ranges::range R>
    struct range_iterator_hash {
        size_t operator()(const std::ranges::iterator_t<R> &i) const {
            return std::hash<int*>()(&*i);
        }
    };
}

#endif //OX_LIB__HELPER_ITERATORS_H
