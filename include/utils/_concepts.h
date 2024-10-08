#ifndef OXLIB__CONCEPTS_H
#define OXLIB__CONCEPTS_H

#define OX_CHAINED_CONCEPTS(namespace1, type_transformation, namespace2, concept_name) \
template<typename T>\
concept type_transformation##_##concept_name = namespace2::concept_name<namespace1::type_transformation##_t<T>>;

#include <concepts>
#include <type_traits>

namespace ox {
    OX_CHAINED_CONCEPTS(std, remove_reference, std, integral);
}

#endif // OXLIB__CONCEPTS_H
