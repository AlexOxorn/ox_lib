//
// Created by alexoxorn on 2021-12-22.
//

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
            snprintf(message, 100, "Invalid dimensions: %zu x %zu and %zu and %zu for operator %c",
                   a.get_width(), a.get_height(), b.get_width(), b.get_height(), op);
        }

        [[nodiscard]] const char * what () const noexcept override {
            return message;
        }
    };

    template <typename T, typename Container = std::vector<T>>
    class matrix : public grid<T, Container> {
        using grid<T, Container>::grid;

    public:
        matrix& operator+=(const matrix& other) {
            if (this->get_dimensions() != other.get_dimensions()) {
                throw invalid_matrix_dimensions(*this, other, '+');
            }
            for (auto sum_iter = this->data.begin(), other_iter = other.data.begin(); sum_iter != this->data.end(); sum_iter++, other_iter++) {
                *(sum_iter) += other_iter;
            }
        }

        matrix operator+(const matrix& other) {
            matrix sum = *this;
            sum += other;
            return sum;
        }

        matrix operator*(const matrix& other) {
            if (this->get_dimensions() != std::make_pair(other.get_height(), other.get_width())) {
                throw invalid_matrix_dimensions(*this, other, '+');
            }
            int new_width = other.get_width();
            auto new_input = std::views::iota(std::size_t(0), other.get_width() * this->get_height())
                   | std::views::transform([this, new_width, &other](std::size_t index) -> int {
                         std::pair coord = std::make_pair(index % new_width, index / new_width);
                         int sum = 0;
                         for(int i : stdv::iota(std::size_t(0), this->get_width())) {
                             sum += this->get(i, coord.second) * other.get(coord.first, i);
                         }
                         return sum;
                     });
            return matrix(new_width, new_input);
        }

        template <typename Scalar> requires requires (T t, Scalar s) { { t * s } -> std::convertible_to<T>; }
        matrix& operator*=(const Scalar& i) {
            std::transform(this->data.begin(), this->data.end(), this->data.begin(), [&i](T a) { return a * i; });
            return *this;
        }

        template <typename Scalar, typename ...TemplateArgs> requires requires (T t, Scalar s) { { t * s } -> std::convertible_to<T>; }
        friend matrix<TemplateArgs...> operator*(const Scalar& s, const matrix<TemplateArgs...>& m) {
            auto cpy = m;
            cpy *= s;
            return cpy;
        }

        void print_matrix() requires std::integral<T> {
            T largest = stdr::max(this->data);
            int width = numberOfDigits(largest);
            this->leveled_foreach(
                   [width](T i) { std::cout << std::setw(width+1) << i; },
                   []() { std::cout << '\n'; }
            );
        }
    };
}


#endif // ADVENTOFCODE__MATRIX_H
