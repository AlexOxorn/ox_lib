#ifndef OX_LIB_HELPER_ITERATORS_H
#define OX_LIB_HELPER_ITERATORS_H

#include <functional>

namespace ox {
    template<typename T>
    class predicateCounter {
        size_t& count;
        std::function<bool(const T&)> predicate;
    public:
        predicateCounter(std::function<bool(T)> pred, size_t& counter) : count(counter), predicate(std::move(pred)) {}
        predicateCounter(size_t& counter) : count(counter), predicate([](const T&) { return true; }) {}

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
        size_t operator()(const typename R::const_iterator &i) const {
            return std::hash<const typename R::value_type *>()(&*i);
        }
    };

    template <std::input_or_output_iterator I>
    struct iterator_hash {
        size_t operator()(const I &i) const {
            return std::hash<const typename I::value_type *>()(&*i);
        }
    };
}

#endif //OX_LIB_HELPER_ITERATORS_H
