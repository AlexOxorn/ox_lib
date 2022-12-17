//
// Created by alexoxorn on 12/16/22.
//

#pragma once

#include <functional>

namespace ox {
    template <typename Left = void, typename Right = void>
    struct pair_hash {
        std::size_t operator()(const std::pair<Left, Right>& c) const {
            return 2 * std::hash<Left>()(c.first) + 3 * std::hash<int>()(c.second);
        }
    };

    template <>
    struct pair_hash<void, void> {
        template <typename Left, typename Right>
        std::size_t operator()(const std::pair<Left, Right>& c) const {
            return 2 * std::hash<Left>()(c.first) + 3 * std::hash<int>()(c.second);
        }
    };
} // namespace ox

