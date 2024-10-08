//
// Created by alexoxorn on 10/7/24.
//

#include "math/_system_solver.h"
#include <ox/future/generator.h>
#include <utility>
#include <vector>
#include <algorithm>

namespace ox::system {
    const std::function<long(long, long)> op_fun[]{
            std::equal_to(), std::plus(), std::minus(), std::multiplies(), std::divides()};
    const std::function<long(long, long)> inv_op_fun[]{
            std::not_equal_to(), std::minus(), std::plus(), std::divides(), std::multiplies()};

    using node_type = std::variant<long, op, std::string>;
    using math_tree = ox::binary_tree<node_type>;
    using math_tree_node = ox::binary_tree<node_type>::node;

    bool reduce(math_tree_node& tree) {
        if (std::holds_alternative<long>(tree.value) or std::holds_alternative<std::string>(tree.value)) {
            return true;
        }

        bool left_reduce = reduce(*tree.children.first);
        bool right_reduce = reduce(*tree.children.second);

        if (!(left_reduce && right_reduce)) {
            return false;
        }

        if (tree.children.first->value.index() == 0 and tree.children.second->value.index() == 0) {
            auto func = op_fun[int8_t(std::get<op>(tree.value))];
            tree.value =
                    func(std::get<long>(tree.children.first->value), std::get<long>(tree.children.second->value));
            tree.children.first.reset();
            tree.children.second.reset();
        } else {
            // special case 0 * _
            if (tree.value == node_type(op::MUL)
                and (tree.get_left_child().value == node_type(0) or tree.get_right_child().value == node_type(0))) {
                tree.value = 0;
                tree.children.first.reset();
                tree.children.second.reset();
            }

            // special case 0 + _
            if (tree.value == node_type(op::PLUS) and tree.get_left_child().value == node_type(0)) {
                auto tmp = tree.get_right_child();
                tree = tmp;
            }
            if (tree.value == node_type(op::PLUS) and tree.get_right_child().value == node_type(0)) {
                auto tmp = tree.get_left_child();
                tree = tmp;
            }

            // special case 1 * _
            if (tree.value == node_type(op::MUL) and tree.get_left_child().value == node_type(1)) {
                auto tmp = tree.get_right_child();
                tree = tmp;
            }
            if (tree.value == node_type(op::MUL) and tree.get_right_child().value == node_type(1)) {
                auto tmp = tree.get_left_child();
                tree = tmp;
            }

            // special case _ - -x
            if (tree.value == node_type(op::MIN) and std::holds_alternative<long>(tree.get_right_child().value)) {
                auto& temp = std::get<long>(tree.get_right_child().value);
                if (temp < 0) {
                    temp *= -1;
                    tree.value = op::PLUS;
                }
            } else if (tree.value == node_type(op::PLUS) and std::holds_alternative<long>(tree.get_right_child().value)) {
                auto& temp = std::get<long>(tree.get_right_child().value);
                if (temp < 0) {
                    temp *= -1;
                    tree.value = op::MIN;
                }
            }
        }
        return true;
    }

    bool expand_division(math_tree_node& lhs, math_tree_node& rhs) {
        if (lhs.value == node_type(op::DIV) and rhs.value != node_type(op::DIV)) {
            rhs.emplace_right(rhs.value);
            rhs.emplace_left(std::move(*lhs.children.second));
            rhs.value = op::MUL;
            lhs = math_tree_node{*lhs.children.first};
            return true;
        } else if (rhs.value == node_type(op::DIV) and lhs.value != node_type(op::DIV)) {
            lhs.emplace_right(lhs.value);
            lhs.emplace_left(std::move(*rhs.children.second));
            lhs.value = op::MUL;
            rhs = math_tree_node{*rhs.children.first};
            return true;
        } else if (rhs.value == node_type(op::DIV) and lhs.value == node_type(op::DIV)) {
            std::swap(rhs.get_right_child(), lhs.get_right_child());
            lhs.value = op::MUL;
            rhs.value = op::MUL;
            return true;
        }
        return false;
    }

    bool expand_multiplication(math_tree_node& tree) {
        if (tree.value == node_type(op::MUL)
            and (tree.get_right_child().value == node_type(op::PLUS)
                 or tree.get_right_child().value == node_type(op::MIN))) {
            auto& multiplier = tree.get_left_child();
            auto& term = tree.get_right_child();
            auto& plus_left = term.get_left_child();
            auto& plus_right = term.get_right_child();
            plus_left.emplace_right(plus_left);
            plus_left.value = op::MUL;
            plus_left.emplace_left(multiplier);
            plus_right.emplace_right(plus_right);
            plus_right.value = op::MUL;
            plus_right.emplace_left(multiplier);

            tree.value = term.value;
            auto left_cpy = plus_left;
            auto right_cpy = plus_right;
            tree.emplace_left(std::move(left_cpy));
            tree.emplace_right(std::move(right_cpy));
            return true;
        } else if (tree.value == node_type(op::MUL)
                   and (tree.get_left_child().value == node_type(op::PLUS)
                        or tree.get_left_child().value == node_type(op::MIN))) {
            auto& multiplier = tree.get_right_child();
            auto& term = tree.get_left_child();
            auto& plus_left = term.get_left_child();
            auto& plus_right = term.get_right_child();
            plus_left.emplace_right(plus_left);
            plus_left.value = op::MUL;
            plus_left.emplace_left(multiplier);
            plus_right.emplace_right(plus_right);
            plus_right.value = op::MUL;
            plus_right.emplace_left(multiplier);

            tree.value = term.value;
            auto left_cpy = plus_left;
            auto right_cpy = plus_right;
            tree.emplace_left(std::move(left_cpy));
            tree.emplace_right(std::move(right_cpy));
            return true;
        }
        if (std::holds_alternative<op>(tree.value)) {
            return expand_multiplication(tree.get_left_child()) or expand_multiplication(tree.get_right_child());
        }
        return false;
    }

    bool contains(math_tree_node& tree, std::string name) {
        if (std::holds_alternative<op>(tree.value)) {
            return contains(tree.get_left_child(), name) or contains(tree.get_right_child(), name);
        }
        return std::holds_alternative<std::string>(tree.value) and std::get<std::string>(tree.value) == name;
    }

    using term_op = std::pair<math_tree_node, op>;

    stdfuture::generator<std::pair<math_tree_node, op>> get_terms(math_tree_node& lhs, op retop = op::PLUS) {
        if (lhs.value == node_type(op::PLUS) || lhs.value == node_type(op::MIN)) {
            for (auto left : get_terms(lhs.get_left_child(), retop)) {
                co_yield left;
            }
            for (auto right :
                 get_terms(lhs.get_right_child(), std::get<op>(lhs.value) == op::MIN ? inv_op[int8_t(retop)] : retop)) {
                co_yield right;
            }
        } else {
            if (lhs.value != node_type(0))
                co_yield {lhs, retop};
        }
    }

    void factor_remove(math_tree_node& lhs, std::string name) {
        if (lhs.value == node_type(op::PLUS) || lhs.value == node_type(op::MIN)) {
            factor_remove(lhs.get_left_child(), name);
            factor_remove(lhs.get_right_child(), name);
        } else {
            if (lhs.value == node_type(name)) {
                lhs.value = 1;
                return;
            }
            if (lhs.get_left_child().value == node_type(name)) {
                auto tmp = lhs.get_right_child();
                lhs = tmp;
                return;
            }
            if (lhs.get_right_child().value == node_type(name)) {
                auto tmp = lhs.get_left_child();
                lhs = tmp;
                return;
            }
            if (lhs.get_left_child().value == node_type(op::MUL)) {
                factor_remove(lhs.get_left_child(), name);
            }
            if (lhs.get_right_child().value == node_type(op::MUL)) {
                factor_remove(lhs.get_right_child(), name);
            }
        }
    }

    bool factor(math_tree_node& lhs, std::string name) {
        if (!std::ranges::all_of(
                    get_terms(lhs), [&name](math_tree_node x) { return contains(x, name); }, &term_op::first))
            return false;

        factor_remove(lhs, name);
        lhs.emplace_right(lhs);
        lhs.emplace_left(name);
        lhs.value = op::MUL;
        return true;
    }

    void print_tree(math_tree_node& tree, std::ostream& os) {
        if (tree.children.first) {
            os << '(';
            print_tree(*tree.children.first);
        }
        std::visit(ox::overload{[&os](auto l) { os << l; },
                                [&os](op c) {
                                    os << op_str[int8_t(c)];
                                }},
                   tree.value);
        if (tree.children.second) {
            print_tree(*tree.children.second);
            os << ')';
        }
    }

    void print_pair(math_tree_node& lhs, math_tree_node& rhs, std::ostream& os = std::cout) {
        print_tree(lhs, os);
        os << " = ";
        print_tree(rhs, os);
        os << std::endl;
    }

    void isolate(math_tree_node& lhs, math_tree_node& rhs, std::string name) {
        std::cout << "EXPAND DIVISION START\n";
        expand_division(lhs, rhs);
        print_pair(lhs, rhs);

        std::cout << "MULTIPLICATION EXPANSION START\n";
        while (expand_multiplication(lhs))
            ;
        while (expand_multiplication(rhs))
            ;
        print_pair(lhs, rhs);

        std::cout << "MOVE NAMED TERMS START\n";
        auto left_terms_s = get_terms(lhs);
        std::vector<std::pair<math_tree_node, op>> left_terms;
        std::ranges::for_each(left_terms_s, [&](auto s) { left_terms.push_back(std::move(s)); });
        auto left_part_point = std::ranges::stable_partition(
                left_terms, [name](auto& x) { return contains(x, name); }, &term_op::first);

        auto right_terms_s = get_terms(rhs);
        std::vector<std::pair<math_tree_node, op>> right_terms;
        std::ranges::for_each(right_terms_s, [&](auto s) { right_terms.push_back(std::move(s)); });
        auto right_part_point = std::ranges::stable_partition(
                right_terms, [name](auto& x) { return contains(x, name); }, &term_op::first);

        math_tree_node new_lhs(op::EQUAL);
        math_tree_node new_rhs(op::EQUAL);

        // left named
        for (auto& [n, pos] : std::ranges::subrange(left_terms.begin(), left_part_point.begin())) {
            if (new_lhs.value == node_type(op::EQUAL)) {
                if (pos == op::MIN) {
                    new_lhs.emplace_right(n);
                    new_lhs.emplace_left(0);
                    new_lhs.value = op::MIN;
                } else {
                    new_lhs = n;
                }
                continue;
            }
            new_lhs.emplace_left(new_lhs);
            new_lhs.emplace_right(n);
            new_lhs.value = pos;
        }

        // right unnamed
        for (auto& [n, pos] : right_part_point) {
            if (new_rhs.value == node_type(op::EQUAL)) {
                if (pos == op::MIN) {
                    new_rhs.emplace_right(n);
                    new_rhs.emplace_left(0);
                    new_rhs.value = op::MIN;
                } else {
                    new_rhs = n;
                }
                continue;
            }
            new_rhs.emplace_left(new_rhs);
            new_rhs.emplace_right(n);
            new_rhs.value = pos;
        }

        // right named
        for (auto& [n, pos] : std::ranges::subrange(right_terms.begin(), right_part_point.begin())) {
            if (new_lhs.value == node_type(op::EQUAL)) {
                if (pos == op::PLUS) {
                    new_lhs.emplace_right(n);
                    new_lhs.emplace_left(0);
                    new_lhs.value = op::MIN;
                } else {
                    new_lhs = n;
                }
                continue;
            }
            new_lhs.emplace_left(new_lhs);
            new_lhs.emplace_right(n);
            new_lhs.value = inv_op[int8_t(pos)];
        }

        // left unnamed
        for (auto& [n, pos] : left_part_point) {
            if (new_rhs.value == node_type(op::EQUAL)) {
                if (pos == op::PLUS) {
                    new_rhs.emplace_right(n);
                    new_rhs.emplace_left(0);
                    new_rhs.value = op::MIN;
                } else {
                    new_rhs = n;
                }
                continue;
            }
            new_rhs.emplace_left(new_rhs);
            new_rhs.emplace_right(n);
            new_rhs.value = inv_op[int8_t(pos)];
        }

        lhs = new_lhs;
        rhs = new_rhs;

        print_pair(lhs, rhs);

        std::cout << "FACTORING START\n";
        if (!factor(lhs, name))
            return;

        print_pair(lhs, rhs);

        std::cout << "FINAL ISOLATION START\n";
        rhs.emplace_left(rhs);
        rhs.emplace_right(lhs.get_right_child());
        rhs.value = op::DIV;
        lhs.value = lhs.get_left_child().value;
        lhs.children.first.reset();
        lhs.children.second.reset();

        print_pair(lhs, rhs);
        std::cout << "FINAL REDUCE START\n";
        reduce(lhs);
        reduce(rhs);
        print_pair(lhs, rhs);
        std::cout << std::endl;
    }
} // namespace ox::system
