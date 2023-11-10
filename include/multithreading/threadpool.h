//
// Created by alexoxorn on 11/8/23.
//

#ifndef OXLIB_THREADPOOL_H
#define OXLIB_THREADPOOL_H

#include "threadsafe_queue.h"
#include "joiner.h"
#include <atomic>
#include <algorithm>
#include <thread>
#include <future>
#include <functional>

namespace ox {
    class thread_pool {
        mutable std::mutex mutex;
        std::condition_variable condition_variable;
        std::vector<std::thread> threads;
        std::atomic_bool shutdown_requested;
        std::queue<std::move_only_function<void()>> queue;
        ox::joiner<std::vector<std::thread>> unique_join;

        bool no_tasks() {
            return shutdown_requested || !queue.empty();
        }

        bool tasks_exists() {
            return !shutdown_requested || (shutdown_requested && !queue.empty());
        }

        class thread_worker {
            thread_pool* pool;
        public:
            explicit thread_worker(thread_pool* pool)
                : pool{pool} {}

            void operator()() {
                std::unique_lock<std::mutex> lock(pool->mutex);
                while (pool->tasks_exists()) {
                    pool->condition_variable.wait(lock, [this] { return pool->no_tasks(); });
                    if (!pool->queue.empty()) {
                        auto func = std::move(pool->queue.front());
                        pool->queue.pop();
                        lock.unlock();
                        func();
                        lock.lock();
                    }
                }
            }
        };

    public:
        explicit thread_pool(size_t size=std::thread::hardware_concurrency()) : unique_join(threads) {
            threads.reserve(size);
            for (int i = 0; i < static_cast<int>(size); ++i) {
                threads.emplace_back(thread_worker(this));
            }
        }
        ~thread_pool() {
            stop();
            condition_variable.notify_all();
        }

        void stop() {
            shutdown_requested = true;
        }

        template <typename F, typename... Args>
        auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
            auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            auto task = std::packaged_task<void()>(func);
            auto future = task.get_future();
            auto wrapper_func = [task = std::move(task)] mutable { std::move(task)(); };
            {
                std::lock_guard lock(mutex);
                queue.push(std::move(wrapper_func));
                condition_variable.notify_one();
            };
            return future;
        }

        void join() {
            unique_join.join();
        }
    };

} // namespace ox

#endif // OXLIB_THREADPOOL_H
