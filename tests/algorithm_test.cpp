#include <ox/algorithms.h>
#include <cstdio>
#include <vector>
#include <list>
#include <forward_list>
#include <string>
#include "algorithm_test.h"

void join_test() {
    const std::list<const char *> sentence1{"Hello", "my", "name", "is", "alex"};
    std::string sentence2{};
    ox::join(
        sentence1.begin(),
        sentence1.end(),
        ",",
        sentence2
    );
    printf("%s!\n", sentence2.data());

    std::vector<std::basic_string<int>> num_seq{{2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4, 4}, {5, 5, 5, 5, 5}};
    std::basic_string<int> num_seq2{};
    ox::join(
        num_seq.begin(),
        num_seq.end(),
        std::basic_string{1},
        num_seq2
    );
    print_numbers(num_seq2);

    std::vector<int> int_vect{514, 951, 4902};
    std::string to_str_join{"Here is my number: "};
    ox::join(
        int_vect.begin(),
        int_vect.end(),
        "-",
        to_str_join,
        static_cast<std::string(*)(int)>(std::to_string)
    );

    printf("%s\n", to_str_join.data());
}

void flatten_test() {
    using superlist = std::list<std::vector<std::forward_list<int>>>;
    superlist deep{
        {{1, 1, 1},{2, 2, 2},{3, 3, 3}},
        {{4, 4, 4},{5, 5, 5},{6, 6, 6}},
        {{7, 7, 7},{8, 8, 8},{9, 9, 9}}
    };
    std::vector<std::forward_list<int>> shallow{{1, 1, 1},{2, 2, 2},{3, 3, 3}};
    std::vector<int> out{};
    ox::deep_flatten(deep.begin(), deep.end(), std::back_inserter(out));
    ox::flatten(shallow.begin(), shallow.end(), std::back_inserter(out));
    print_numbers(out);
}

