//
// Created by alexoxorn on 10/5/24.
//

#ifndef OXLIB_RATIONAL_H
#define OXLIB_RATIONAL_H

#include <cmath>

namespace ox {
    struct div_by_zero_error : std::overflow_error {
        using std::overflow_error::overflow_error;
    };

    struct rational {
        long numerator;
        long denominator;

        constexpr rational() : numerator{0}, denominator{1} {}
        constexpr rational(long n) : numerator{n}, denominator{1} {}
        constexpr rational(long n, long m) : numerator{n}, denominator{m} {
            if (n == 0)
                denominator = 1;
            else if (m == 0)
                throw div_by_zero_error("Divide By Zero");
            else
                reduce();
        }
    private:
        constexpr void reduce() {
            auto r = std::gcd(numerator, denominator);
            numerator /= r;
            denominator /= r;
        }
    public:
        constexpr rational operator+(const rational& other) const {
            auto new_denominator = std::lcm(denominator, other.denominator);
            auto new_numerator = numerator * std::gcd(denominator, new_denominator)
                               + other.numerator * std::gcd(other.denominator, new_denominator);
            return {new_numerator, new_denominator};
        }

        constexpr rational operator-() const { return {-numerator, denominator}; }

        constexpr rational operator-(const rational& other) const { return *this + (-other); }

        constexpr rational operator*(const rational& other) const {
            auto new_numerator = numerator * other.numerator;
            auto new_denominator = denominator * other.denominator;
            return {new_numerator, new_denominator};
        }

        constexpr rational operator/(const rational& other) const {
            return *this * rational(other.denominator, other.numerator);
        }

        template <std::floating_point F>
        explicit operator F() const {
            return F(numerator) / F(denominator);
        }
        template <std::integral F>
        explicit operator F() const {
            return numerator / denominator;
        }

        constexpr bool operator==(const rational& other) const = default;
        constexpr auto operator<=>(const rational& other) const {
            return double(*this) <=> double(other);
        }

        //    constexpr auto operator<=>(const rational& other) const {
        //        auto common_denominator = std::lcm(denominator, other.denominator);
        //        return numerator * std::gcd(denominator, common_denominator)
        //           <=> other.numerator * std::gcd(other.denominator, common_denominator);
        //    }
    };
}

#endif // OXLIB_RATIONAL_H
