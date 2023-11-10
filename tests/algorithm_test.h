#include <ox/algorithms.h>
#include <cstdio>
#include <vector>
#include <list>
#include <forward_list>
#include <string>
#include <iterator>
#include <ranges>
#include <concepts>

template<std::ranges::range value>
requires std::same_as<int, typename value::value_type>
void print_numbers(value nums) {
    for (auto num : nums) {
        printf("%d ", num);
    }
    printf("\n");
}

template<std::ranges::range value>
requires std::same_as<unsigned int, typename value::value_type>
void print_numbers(value nums) {
    for (auto num : nums) {
        printf("%u ", num);
    }
    printf("\n");
}

template<std::ranges::range value>
requires std::same_as<double, typename value::value_type>
void print_numbers(value nums) {
    for (auto num : nums) {
        printf("%lg ", num);
    }
    printf("\n");
}

template<std::ranges::range value>
requires std::same_as<std::string, typename value::value_type>
void print_numbers(value nums) {
    for (auto num : nums) {
        printf("%s ", num.c_str());
    }
    printf("\n");
}

void join_test();
void flatten_test();

