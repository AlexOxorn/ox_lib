#pragma once
#include <ranges>

namespace ox::ranges {
    template<class T>
    class possibly_owning_view {
        using type = std::ranges::owning_view<T>;
    };
    template<class T>
    class possibly_owning_view<T&> {
        using type = std::ranges::views::all_t<T&> ;
    };

    template <class T>
    using possibly_owning_view_t = possibly_owning_view<T>::type;
}
