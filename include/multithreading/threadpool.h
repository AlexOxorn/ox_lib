#ifndef OXLIB_THREADPOOL_H
#define OXLIB_THREADPOOL_H

#include "threadsafe_queue.h"
#include "joiner.h"
#include <atomic>
#include <algorithm>
#include <thread>

namespace ox {
    template <std::invocable F>
    class thread_pool {
        std::atomic_bool done;
        ox::safe_queue<F> work_queue;
        std::vector<std::jthread> threads;

        void worker_thread() {
            while (!done) {
                F task;
                if (auto f = work_queue.try_pop(); f) {
                    (*f)();
                } else {
                    std::this_thread::yield();
                }
            }
        }
    public:
        thread_pool() : done{false}, threads(std::thread::hardware_concurrency()) {
            try {
                std::ranges::generate(threads, [this]() { return std::jthread(&thread_pool::worker_thread, this); });
            } catch (...) {
                done = true;
                throw;
            }
        }
        ~thread_pool() { done = true; }
        void submit(F f) { work_queue.push(std::move(f)); }
    };
} // namespace ox

#endif // OXLIB_THREADPOOL_H
