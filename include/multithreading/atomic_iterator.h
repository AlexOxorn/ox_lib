//
// Created by alexoxorn on 11/10/23.
//

#ifndef OXLIB_ATOMIC_ITERATOR_H
#define OXLIB_ATOMIC_ITERATOR_H

#include <iterator>
#include <mutex>
#include <optional>

namespace ox {
    template<std::input_iterator IT, std::sentinel_for<IT> S>
    class atomic_iterator {
        using value_type = IT::value_type;
        std::mutex increment_mutex;
        IT iterable;
        S end;
        long iteration_count = 0;

    public:
        atomic_iterator(atomic_iterator&&) = default;
        atomic_iterator(const atomic_iterator&) = delete;
        atomic_iterator& operator=(atomic_iterator&&) = default;
        atomic_iterator& operator=(const atomic_iterator&) = delete;

        std::pair<std::optional<value_type>, long> get_and_increment() {
            std::lock_guard lock(increment_mutex);
            if (iterable == end)
                return {std::nullopt, -1};
            auto x = *iterable++;
            return {std::optional{x}, ++iteration_count};
        }

        atomic_iterator(IT iter, S sent)
            : iterable{iter},
              end{sent} {}
    };
} // namespace ox

#endif // OXLIB_ATOMIC_ITERATOR_H
