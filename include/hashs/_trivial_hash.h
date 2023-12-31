#pragma once

#include <functional>
#include <type_traits>

namespace ox {
    struct trivial_hash {
        template <typename T>
        requires std::has_unique_object_representations_v<T>
        size_t operator()(const T& data) const {
            std::string_view ss((char*) &data, sizeof(data));
            return std::hash<std::string_view>()(ss);
        }
    };
} // namespace ox
