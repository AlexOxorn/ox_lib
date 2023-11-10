//
// Created by alexoxorn on 11/8/23.
//

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
        void join() {
            for (auto& c : *container) {
                if (c.joinable())
                    c.join();
            }
        }

        ~joiner() {
            join();
        }
        explicit joiner(C& _container): container{&_container} {};
    };
}

#endif // ADVENTOFCODE_JOINER_H
