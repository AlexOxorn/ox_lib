#ifndef OX_JOINER_H
#define OX_JOINER_H

#include <ranges>

namespace ox {

    template <std::ranges::forward_range C> requires requires (C::value_type v) {
        { v.join() } -> std::same_as<void>;
    }
    class joiner {
        C* container;
    public:
        ~joiner() {
            for (auto& c : *container) {
                c.join();
            }
        }
        explicit joiner(C& _container): container{&_container} {};
    };
}

#endif // ADVENTOFCODE_JOINER_H
