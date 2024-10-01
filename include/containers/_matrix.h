#ifndef ADVENTOFCODE__MATRIX_H
#define ADVENTOFCODE__MATRIX_H

#include "_2d_grid.h"
#include <ox/math.h>
#include <ranges>
#include <exception>
#include <cmath>
#include <algorithm>
#include <concepts>

namespace ox {

    template <typename T, typename Container>
    class matrix;

    struct invalid_matrix_dimensions : public std::exception {
        char message[120]{};

        invalid_matrix_dimensions(const auto& a, const auto& b, char op) {
            snprintf(message,
                     100,
                     "Invalid dimensions: %zu x %zu and %zu x %zu for operator %c",
                     a.get_width(),
                     a.get_height(),
                     b.get_width(),
                     b.get_height(),
                     op);
        }

        invalid_matrix_dimensions(const auto& a, const auto& b, const auto& dest, char op) {
            snprintf(message,
                     100,
                     "Invalid dimensions: %zu x %zu and %zu x %zu for operator %c to %zu x %zu",
                     a.get_width(),
                     a.get_height(),
                     b.get_width(),
                     b.get_height(),
                     op,
                     dest.get_width(),
                     dest.get_height());
        }

        [[nodiscard]] const char* what() const noexcept override { return message; }
    };

    template <typename T, typename Container = std::vector<T>>
    class matrix : public grid<T, Container> {
        using grid<T, Container>::grid;

        static auto get_multiplication_stream(const matrix& lhs, const matrix& rhs) {
            return std::views::iota(std::size_t(0), rhs.get_width() * lhs.get_height())
                 | std::views::transform([&](std::size_t index) {
                       std::pair coord = std::make_pair(index % rhs.get_width(), index / rhs.get_width());
                       T sum{};
                       for (int i : stdv::iota(std::size_t(0), lhs.get_width())) {
                           auto prod = lhs.at(i, coord.second) * rhs.at(coord.first, i);
                           sum += prod;
                       }
                       return sum;
                   });
        }
    public:
        constexpr matrix& operator+=(const matrix& other) {
            if (this->get_dimensions() != other.get_dimensions()) {
                throw invalid_matrix_dimensions(*this, other, '+');
            }
            std::transform(this->data.begin(),
                           this->data.end(),
                           other.data.begin(),
                           this->data.begin(),
                           [](const auto& a, const auto& b) { return a + b; });
            return *this;
        }
        constexpr matrix operator+(const matrix& other) const {
            matrix sum = *this;
            sum += other;
            return sum;
        }
        constexpr matrix& operator-=(const matrix& other) {
            if (this->get_dimensions() != other.get_dimensions()) {
                throw invalid_matrix_dimensions(*this, other, '-');
            }
            std::transform(this->data.begin(),
                           this->data.end(),
                           other.data.begin(),
                           this->data.begin(),
                           [](const auto& a, const auto& b) { return a - b; });
            return *this;
        }

        constexpr matrix operator-(const matrix& other) const {
            matrix sum = *this;
            sum -= other;
            return sum;
        }

        constexpr matrix operator*(const matrix& other) const {
            if (this->get_width() != other.get_height()) {
                throw invalid_matrix_dimensions(*this, other, '*');
            }
            int new_width = other.get_width();
            auto new_input = get_multiplication_stream(*this, other);
            return matrix(new_width, new_input);
        }

        template <typename Scalar>
        requires requires(T t, Scalar s) {
            { t* s } -> std::convertible_to<T>;
        }
        constexpr matrix& operator*=(const Scalar& i) {
            std::transform(this->data.begin(), this->data.end(), this->data.begin(), [&i](T a) { return a * i; });
            return *this;
        }

        template <typename Scalar, typename... TemplateArgs>
        requires requires(T t, Scalar s) {
            { t* s } -> std::convertible_to<T>;
        }
        constexpr inline friend matrix<TemplateArgs...> operator*(const Scalar& s, const matrix<TemplateArgs...>& m) {
            auto cpy = m;
            cpy *= s;
            return cpy;
        }

        template <typename Scalar>
        requires requires(T t, Scalar s) {
            { t / s } -> std::convertible_to<T>;
        }
        constexpr matrix& operator/=(const Scalar& i) {
            std::transform(this->data.begin(), this->data.end(), this->data.begin(), [&i](T a) { return a / i; });
            return *this;
        }

        template <typename Scalar, typename... TemplateArgs>
        requires requires(T t, Scalar s) {
            { t / s } -> std::convertible_to<T>;
        }
        constexpr inline matrix operator/(const Scalar& s) {
            auto cpy = *this;
            cpy /= s;
            return cpy;
        }

        constexpr static void in_place_multiplication(matrix& dest, const matrix& lhs, const matrix& rhs) {
            if (lhs.get_width() != rhs.get_height() && dest.get_width() == rhs.get_width()
                && dest.get_height() == lhs.get_height()) {
                throw invalid_matrix_dimensions(lhs, rhs, dest, '*');
            }
            auto new_input = get_multiplication_stream(lhs, rhs);
            std::copy(new_input.begin(), new_input.end(), dest.data.begin());
        }

        constexpr matrix transpose() const {
            auto new_input = std::views::iota(std::size_t(0), this->get_size())
                           | std::views::transform([this](std::size_t index) -> int {
                                 return this->get(index / this->get_height(), index % this->get_height());
                             });
            return matrix(this->get_height(), new_input);
        }

        void print_matrix()
        requires std::integral<T>
        {
            auto x = this->data | std::views::transform([](auto x) { return numberOfDigits(std::abs(x)) + (x < 0); });
            print_matrix(std::ranges::max(x));
        }

        void print_matrix(int width)
        requires std::integral<T>
        {
            this->leveled_foreach([width](T i) { std::cout << std::setw(width + 1) << i; },
                                  []() { std::cout << '\n'; });
        }

        void print_matrix() {
            this->leveled_foreach([](T i) { std::cout << i << ", "; }, []() { std::cout << '\n'; });
        }
    };
} // namespace ox

#endif // ADVENTOFCODE__MATRIX_H
