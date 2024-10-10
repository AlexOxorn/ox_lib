//
// Created by alexoxorn on 10/7/24.
//

#ifndef OXLIB_SYSTEM_SOLVER_H
#define OXLIB_SYSTEM_SOLVER_H

#include <utility>
#include <variant>
#include <vector>
#include <concepts>
#include <iostream>
#include <functional>
#include <ox/tree.h>
#include <ox/utils.h>

namespace ox {
    namespace system {
        enum class op : int8_t {
            EQUAL,
            PLUS,
            MIN,
            MUL,
            DIV,
        };

        struct term;
        struct quotient;

        struct term {
            using valtype = std::variant<long, std::string, term>;

            struct value_threeway_comp {
                static std::partial_ordering operator()(const valtype& lhs, const valtype& rhs) {
                    auto i1 = lhs.index();
                    auto i2 = rhs.index();
                    if (i1 != i2) {
                        return i1 <=> i2;
                    }
                    switch(i1) {
                        case 0: return std::get<0>(lhs) <=> std::get<0>(lhs);
                        case 1: return std::get<1>(lhs) <=> std::get<1>(lhs);
                        case 2: return std::partial_ordering::unordered;
                        default: std::unreachable();
                    }
                }
            };

            std::vector<valtype> factors;
        };

        struct expr {
            std::vector<term> terms;
            std::vector<uint8_t> sign;
        };

        struct quotient {
            std::variant<expr, std::pair<expr, expr>> value;
        };



        constexpr inline char op_str[]{'=', '+', '-', '*', '%'};
        constexpr inline op inv_op[]{op::EQUAL, op::MIN, op::PLUS, op::DIV, op::MUL};
        extern const std::function<long(long, long)> op_fun[];
        extern const std::function<long(long, long)> inv_op_fun[];

        using node_type = std::variant<long, op, std::string>;
        using math_tree_node = ox::binary_tree<node_type>::node;

        bool reduce(math_tree_node& tree);
        void print_tree(const math_tree_node& tree, char o_bracket = 0, char c_bracket = 0,
                        std::ostream& os = std::cout);
        void print_pair(const math_tree_node& lhs, const math_tree_node& rhs, std::ostream& os = std::cout);
        bool expand_division(math_tree_node& lhs, math_tree_node& rhs);
        bool expand_multiplication(math_tree_node& tree);
        bool contains(const math_tree_node& tree, std::string name);
        void factor_remove(math_tree_node& lhs, std::string name);
        bool factor(math_tree_node& lhs, const std::string& name);
        math_tree_node isolate(math_tree_node lhs, math_tree_node rhs, std::string name, bool debug = false);
        math_tree_node substitute(math_tree_node lhs, std::string name, const math_tree_node& with);
        void in_place_substitute(math_tree_node& lhs, std::string name, const math_tree_node& with);

        struct builder {
            math_tree_node tree_node;
            explicit builder(long l) : tree_node(l) {}
            explicit builder(std::string name) : tree_node(std::move(name)) {}

            static builder&& join(builder&& left, builder&& right, op oper) {
                left.tree_node.emplace_left(left.tree_node);
                left.tree_node.emplace_right(right.tree_node);
                left.tree_node.value = oper;
                return std::move(left);
            }

            builder&& operator+(builder&& other) && { return join(std::move(*this), std::move(other), op::PLUS); }
            builder&& operator-(builder&& other) && { return join(std::move(*this), std::move(other), op::MIN); }
            builder&& operator*(builder&& other) && { return join(std::move(*this), std::move(other), op::MUL); }
            builder&& operator/(builder&& other) && { return join(std::move(*this), std::move(other), op::DIV); }

            operator math_tree_node&() & { return tree_node; }
        };
    } // namespace system

} // namespace ox

#endif // OXLIB_SYSTEM_SOLVER_H
