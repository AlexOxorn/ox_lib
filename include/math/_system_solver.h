//
// Created by alexoxorn on 10/7/24.
//

#ifndef OXLIB_SYSTEM_SOLVER_H
#define OXLIB_SYSTEM_SOLVER_H

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

        constexpr inline char op_str[]{'=', '+', '-', '*', '/'};
        constexpr inline op inv_op[]{op::EQUAL, op::MIN, op::PLUS, op::DIV, op::MUL};
        extern const std::function<long(long, long)> op_fun[];
        extern const std::function<long(long, long)> inv_op_fun[];

        using node_type = std::variant<long, op, std::string>;
        using math_tree = ox::binary_tree<node_type>;
        using math_tree_node = ox::binary_tree<node_type>::node;

        bool reduce(math_tree_node& tree);
        void print_tree(math_tree_node& tree, std::ostream& os = std::cout);
        bool expand_division(math_tree_node& lhs, math_tree_node& rhs);
        bool expand_multiplication(math_tree_node& tree);
        bool contains(math_tree_node& tree, std::string name);
        void factor_remove(math_tree_node& lhs, std::string name);
        bool factor(math_tree_node& lhs, std::string name);
        void print_tree(math_tree_node& tree, std::ostream& os);
        void isolate(math_tree_node& lhs, math_tree_node& rhs, std::string name);

        struct builder {
            math_tree_node tree_node;
            builder(long l) : tree_node(l) {}
            builder(std::string name) : tree_node(name) {}

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
        };
    } // namespace system

} // namespace ox

#endif // OXLIB_SYSTEM_SOLVER_H
