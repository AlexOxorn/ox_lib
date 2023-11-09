//
// Created by alexoxorn on 11/8/23.
//

#ifndef OX_THREADSAFE_QUEUE_H
#define OX_THREADSAFE_QUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <optional>

namespace ox {
    template <class T>
    class safe_queue {
    public:
        // Add an element to the queue.
        void push(T t) {
            std::lock_guard<std::mutex> lock(m);
            q.push(t);
            c.notify_one();
        }

        T wait_pop() {
            std::unique_lock<std::mutex> lock(m);
            c.wait(lock, [this]() { return !q.empty(); });
            return true_pop();
        }

        std::optional<T> try_pop() {
            std::unique_lock<std::mutex> lock(m);
            if (q.empty()) {
                return {};
            }
            return true_pop();
        }
    private:
        std::queue<T> q;
        mutable std::mutex m;
        std::condition_variable c;

        T true_pop() {
            T val = q.front();
            q.pop();
            return val;
        }
    };
} // namespace ox

#endif // OX_THREADSAFE_QUEUE_H
