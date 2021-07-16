#ifndef _OXORN_PTR_H
#define _OXORN_PTR_H

#include <type_traits>
#include <concepts>

namespace ox {
    template<typename T, std::integral base_type=u32>
    class ptr {
        base_type addr;
    public:
        ptr() = default;

        T operator*() {
            return *(T*)addr;
        }

        ptr<T, base_type>& operator++() {
            addr += sizeof(T);
            return *this;
        }

        ptr<T, base_type> operator+(int a) {
            return ptr<T>{addr + a};
        }

        ptr<T, base_type> operator-(int a) {
            return ptr<T>{addr - a};
        }

        ptr<T, base_type>& operator+=(int a) {
            addr += a * sizeof(T);
            return *this;
        }

        ptr<T, base_type>& operator-=(int a) {
            addr -= a * sizeof(T);
            return *this;
        }

        T& operator[](int a) {
            return *(T*)(this->addr + sizeof(T) * a);
        }

        bool operator<=>(const ptr<T, base_type>& other) const = default;

        operator T*() {
            return (T*)addr;
        }

        explicit operator base_type() {
            return addr;
        }

        operator bool() {
            return addr != 0;
        }
    };

    template <std::integral base_type>
    class ptr<void, base_type> {
        base_type addr;
    public:
        ptr(base_type pAddr) :addr{pAddr} {};
        ptr() = default;

        operator void*() {
            return (void*)addr;
        }

        explicit operator base_type() {
            return addr;
        }

        bool operator<=>(const ptr<void, base_type>& other) const = default;

        operator bool() {
            return addr != 0;
        }
    };
}


#endif
