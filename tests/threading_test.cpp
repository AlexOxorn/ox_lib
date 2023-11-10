//
// Created by alexoxorn on 11/9/23.
//

#include "threading_test.h"
#include <ox/threading.h>
#include <thread>

void thread_test() {
    /*
     IntStream.range(0, 100)
       .parallel()
       .filter(a -> a % 2 == 0)
       .map(a -> a * a)
       .forEach(System.out::println);
    */

    std::mutex printing_mutex;
    ox::thread_pool pool;

    for (int i = 0; i < 100; ++i) {
        pool.submit([&](int i) {
            if (i % 2 == 0) {
                pool.submit([&](int i) {
                    pool.submit([&](int i) {
                        std::scoped_lock lock(printing_mutex);
                        printf("%d\n", i);
                    }, i * i);
                }, i);
            }
        }, i);
    }
}

void atomic_iterator_test() {
    std::mutex printing_mutex;
    auto x = std::ranges::views::iota(0, 100)
           | std::ranges::views::filter([](int i) { return i % 2 == 0; })
           | std::ranges::views::transform([] (int i) { return i * i; });
    ox::atomic_iterator ato_it(x.begin(), x.end());
    ox::thread_pool pool;

    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        pool.submit([&]() {
            while (true) {
                auto [res, count] = ato_it.get_and_increment();
                if (!res) {
                    return;
                }
                std::scoped_lock lock(printing_mutex);
                printf("%ld -> %d\n", count, *res);
            }
        });
    }
}