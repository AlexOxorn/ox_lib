#pragma once
#include <concepts>
#include <cmath>

#define OP_ASSIGNMENT(op, T, val) \
  T& operator op##=(const T& other) { \
    val op## = other.val; \
    update(); \
    return *this; \
  } \
  T& operator op##=(const std::integral auto& other) { \
    val op## = other; \
    update(); \
    return *this; \
  }
#define OPERATOR_EXPANSION(op, T) \
  T operator op(const T& other) const { \
    T ret{*this}; \
    ret op## = other; \
    return ret; \
  } \
  T operator op(const std::integral auto& other) const { \
    T ret{*this}; \
    ret op## = other; \
    return ret; \
  }

namespace ox {
    template <size_t M, std::integral T = long>
    struct modulo {
        T result;

        modulo() = default;
        modulo(std::integral auto in) : result(in){ update(); };

        OP_ASSIGNMENT(+, modulo, result);
        OPERATOR_EXPANSION(+, modulo)
        OP_ASSIGNMENT(-, modulo, result);
        OPERATOR_EXPANSION(-, modulo)
        OP_ASSIGNMENT(*, modulo, result);
        OPERATOR_EXPANSION(*, modulo)

        modulo& operator++() { return *this += 1; }
        modulo& operator--() { return *this -= 1; }

        modulo operator++(int) {
            modulo ret{*this};
            *this += 1;
            return ret;
        }
        modulo operator--(int) {
            modulo ret{*this};
            *this -= 1;
            return ret;
        }

        operator T() { return result; }
    private:
        void update() {
            result %= M;
            if (result < 0) {
                result += M;
            }
        }
    };

    template <std::integral T>
    T mod(T divisor, T denominator) {
        return divisor - denominator * (T)floor((double)divisor/denominator);
    }

    template <std::integral T> requires std::is_unsigned_v<T>
    T mod(T divisor, T denominator) {
        return divisor % denominator;
    }
} // namespace ox

#undef OP_ASSIGNMENT
#undef OPERATOR_EXPANSION
