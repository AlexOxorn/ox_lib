//
// Created by alexoxorn on 10/7/24.
//

#include "math/_system_solver.h"
#include <ox/future/generator.h>
#include <utility>
#include <format>
#include <vector>
#include <algorithm>
#include <ranges>
#include <set>

#define DEF_SAFE_FUNC(funcname) \
    [](long l, long r) -> std::optional<long> { \
        long res; \
        if (!funcname(l, r, &res)) \
            return res; \
        return std::nullopt; \
    }

namespace ox::system {
    const std::function<long(long, long)> op_fun[]{
            std::equal_to(), std::plus(), std::minus(), std::multiplies(), std::divides()};
    const std::function<std::optional<long>(long, long)> op_safe_fun[]{std::equal_to(),
                                                                       DEF_SAFE_FUNC(__builtin_add_overflow),
                                                                       DEF_SAFE_FUNC(__builtin_sub_overflow),
                                                                       DEF_SAFE_FUNC(__builtin_mul_overflow),
                                                                       [](long l, long r) -> std::optional<long> {
                                                                           if (l % r != 0)
                                                                               return std::nullopt;
                                                                           return l / r;
                                                                       }};
    const std::function<long(long, long)> inv_op_fun[]{
            std::not_equal_to(), std::minus(), std::plus(), std::divides(), std::multiplies()};

    using node_type = std::variant<long, op, std::string>;
    using math_tree = ox::binary_tree<node_type>;
    using math_tree_node = ox::binary_tree<node_type>::node;

    using term_op = std::pair<math_tree_node, op>;

    stdfuture::generator<std::pair<math_tree_node, op>> get_terms(const math_tree_node& lhs, op retop = op::PLUS) {
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

    void get_names(const math_tree_node& m, std::set<std::string>& res) {
        if (std::holds_alternative<std::string>(m.value))
            res.insert(std::get<std::string>(m.value));
        if (std::holds_alternative<op>(m.value)) {
            get_names(m.get_left_child(), res);
            get_names(m.get_right_child(), res);
        }
    }

    std::set<std::string> get_common_names(const math_tree_node& m) {
        std::set<std::string> res;
        auto terms = get_terms(m) | std::views::keys;
        auto head = terms.begin();
        if (head == terms.end())
            return res;
        get_names(*head, res);
        for (; ++head != terms.end();) {
            std::set<std::string> tmp;
            std::set<std::string> next;
            get_names(*head, tmp);
            std::ranges::set_intersection(res, tmp, std::insert_iterator(next, next.begin()));
            std::swap(next, res);
        }
        return res;
    }

    bool contains(const math_tree_node& tree, std::string name) {
        if (std::holds_alternative<op>(tree.value)) {
            return contains(tree.get_left_child(), name) or contains(tree.get_right_child(), name);
        }
        return std::holds_alternative<std::string>(tree.value) and std::get<std::string>(tree.value) == name;
    }

    bool every_term_contains_name(const math_tree_node& lhs, std::string name) {
        return std::ranges::all_of(
                get_terms(lhs),
                [&name](const math_tree_node& x) { return contains(x, name) or x.value == node_type(0); },
                &term_op::first);
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
            if (lhs.value == node_type(0)) {
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

    bool factor(math_tree_node& lhs, const std::string& name) {
        if (!every_term_contains_name(lhs, name))
            return false;

        factor_remove(lhs, name);
        lhs.emplace_right(lhs);
        lhs.emplace_left(name);
        lhs.value = op::MUL;
        return true;
    }

    bool reduce(math_tree_node& tree) {
        if (std::holds_alternative<long>(tree.value) or std::holds_alternative<std::string>(tree.value)) {
            return false;
        }

        bool left_reduce = reduce(*tree.children.first);
        bool right_reduce = reduce(*tree.children.second);
        bool reduced = left_reduce || right_reduce;

        if (tree.children.first->value.index() == 0 and tree.children.second->value.index() == 0) {
            auto func = op_safe_fun[int8_t(std::get<op>(tree.value))];
            auto tmp = func(std::get<long>(tree.children.first->value), std::get<long>(tree.children.second->value));
            if (tmp) {
                tree.value = *tmp;
                tree.children.first.reset();
                tree.children.second.reset();
                reduced |= true;
            }
        } else {
            // dividing common names
            if (tree.value == node_type(op::DIV)) {
                auto right_common_names = get_common_names(tree.get_right_child());
                auto left_common_names = get_common_names(tree.get_left_child());
                std::set<std::string> common;
                std::ranges::set_intersection(
                        left_common_names, right_common_names, std::insert_iterator(common, common.begin()));
                for (const auto& name : common) {
                    factor_remove(tree.get_right_child(), name);
                    factor_remove(tree.get_left_child(), name);
                    reduced |= true;
                }
            }

            // special case name * not_name
            if (tree.value == node_type(op::MUL)
                and (std::holds_alternative<std::string>(tree.get_left_child().value)
                     and !std::holds_alternative<std::string>(tree.get_right_child().value))) {
                auto tmpl = tree.get_left_child();
                auto tmpr = tree.get_right_child();
                tree.emplace_left(tmpr);
                tree.emplace_right(tmpl);
                reduced |= true;
            }

            // special case 0 * _
            // special case _ * 0
            // special case 0 / _
            if (tree.value == node_type(op::MUL)
                and (tree.get_left_child().value == node_type(0) or tree.get_right_child().value == node_type(0))) {
                tree.value = 0;
                tree.children.first.reset();
                tree.children.second.reset();
                reduced |= true;
            }

            if (tree.value == node_type(op::DIV) and tree.get_left_child().value == node_type(0)) {
                tree.value = 0;
                tree.children.first.reset();
                tree.children.second.reset();
                reduced |= true;
            }

            // special case 0 + _
            // special case _ + 0
            // special case _ - 0
            if (tree.value == node_type(op::PLUS) and tree.get_left_child().value == node_type(0)) {
                auto tmp = tree.get_right_child();
                tree = tmp;
                reduced |= true;
            }
            if (tree.value == node_type(op::PLUS) and tree.get_right_child().value == node_type(0)) {
                auto tmp = tree.get_left_child();
                tree = tmp;
                reduced |= true;
            }
            if (tree.value == node_type(op::MIN) and tree.get_right_child().value == node_type(0)) {
                auto tmp = tree.get_left_child();
                tree = tmp;
                reduced |= true;
            }

            // special case [0 - x] / [0 - y]
            if (tree.value == node_type(op::DIV)) {
                if (tree.get_left_child().value == node_type(op::MIN)
                    and tree.get_right_child().value == node_type(op::MIN)) {
                    if (tree.get_left_child().get_left_child().value == node_type(0)
                        and tree.get_right_child().get_left_child().value == node_type(0)) {
                        auto& l = tree.get_left_child();
                        auto& r = tree.get_right_child();

                        auto tmp_l = l.get_right_child();
                        auto tmp_r = r.get_right_child();
                        l = tmp_l;
                        r = tmp_r;
                        reduced |= true;
                    }
                }
            }

            // special case 1 * _
            // special case _ / 1
            // special case _ / -1
            if (tree.value == node_type(op::MUL) and tree.get_left_child().value == node_type(1)) {
                auto tmp = tree.get_right_child();
                tree = tmp;
                reduced |= true;
            }
            if (tree.value == node_type(op::MUL) and tree.get_right_child().value == node_type(1)) {
                auto tmp = tree.get_left_child();
                tree = tmp;
                reduced |= true;
            }
            if (tree.value == node_type(op::DIV) and tree.get_right_child().value == node_type(1)) {
                auto tmp = tree.get_left_child();
                tree = tmp;
                reduced |= true;
            }
            if (tree.value == node_type(op::DIV) and tree.get_right_child().value == node_type(-1)) {
                auto tmp = tree.get_left_child();
                tree.emplace_left(0);
                tree.emplace_right(tmp);
                tree.value = op::MIN;
                reduced |= true;
            }

            // special case _ - -x
            if (tree.value == node_type(op::MIN) and std::holds_alternative<long>(tree.get_right_child().value)) {
                auto& temp = std::get<long>(tree.get_right_child().value);
                if (temp < 0) {
                    temp *= -1;
                    tree.value = op::PLUS;
                    reduced |= true;
                }
            } else if (tree.value == node_type(op::PLUS)
                       and std::holds_alternative<long>(tree.get_right_child().value)) {
                auto& temp = std::get<long>(tree.get_right_child().value);
                if (temp < 0) {
                    temp *= -1;
                    tree.value = op::MIN;
                    reduced |= true;
                }
            }
        }
        return reduced;
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

    void print_tree(const math_tree_node& tree, char o_bracket, char c_bracket, std::ostream& os) {
        if (tree.children.first) {
            if (o_bracket)
                os << o_bracket;
            if (tree.value == node_type(op::DIV))
                print_tree(*tree.children.first, '(', ')', os);
            else if (tree.value == node_type(op::MUL)
                     and (tree.get_left_child().value == node_type(op::PLUS)
                          or tree.get_left_child().value == node_type(op::MIN)))
                print_tree(*tree.children.first, '(', ')', os);
            else
                print_tree(*tree.children.first, 0, 0, os);
        }
        std::visit(ox::overload{[&os](auto l) { os << l; },
                                [&os](op c) {
                                    if (c == op::MUL)
                                        os << op_str[int8_t(c)];
                                    else
                                        os << ' ' << op_str[int8_t(c)] << ' ';
                                }},
                   tree.value);
        if (tree.children.second) {
            if (tree.value == node_type(op::DIV))
                print_tree(*tree.children.second, '(', ')', os);
            else if (tree.value == node_type(op::MUL)
                     and (tree.get_right_child().value == node_type(op::PLUS)
                          or tree.get_right_child().value == node_type(op::MIN)))
                print_tree(*tree.children.second, '(', ')', os);
            else if (tree.value == node_type(op::MIN)
                     and (tree.get_right_child().value == node_type(op::PLUS)
                          or tree.get_right_child().value == node_type(op::MIN)))
                print_tree(*tree.children.second, '(', ')', os);
            else
                print_tree(*tree.children.second, 0, 0, os);
            if (c_bracket)
                os << c_bracket;
        }
    }

    std::string expression_to_string(const term& ex) {
        auto x = ex.factors | std::views::transform([](const term::valtype& t) {
            return std::visit(ox::overload{
                               [](const std::string& s) { return s; },
                               [](long l) { return std::to_string(l); },
                               [](const term& t) { return std::format("({})", expression_to_string(t)); }
                       }, t);
        }) | std::views::join_with(' ') | std::views::common;
        return {x.begin(), x.end()};
    }

    std::string expression_to_string(const expr& ex) {
        std::string to_ret;
        for (auto i = 0zu; i < ex.terms.size(); ++i) {
            auto t = ex.terms[i];
            auto s = ex.sign[i];
            if (i != 0 or s < 0) {
                if (i != 0)
                    to_ret += ' ';
                (to_ret += s < 0 ? '-' : '+') += ' ';
            }
            to_ret += expression_to_string(t);
        }

        return to_ret;
    }

    std::string expression_to_string(const quotient& quote) {
        std::visit(ox::overload{
                [](const expr& ex) { return expression_to_string(ex); },
                [](const std::pair<expr, expr>& ex) {
                    return std::format("[{}] / [{}]", expression_to_string(ex.first), expression_to_string(ex.second));
                }
        }, quote.value);
    }

    void print_pair(const math_tree_node& lhs, const math_tree_node& rhs, std::ostream& os) {
        os << '\t';
        print_tree(lhs, 0, 0, os);
        os << " = ";
        print_tree(rhs, 0, 0, os);
        os << std::endl;
    }

    math_tree_node isolate(math_tree_node lhs, math_tree_node rhs, std::string name, bool debug) {
        if (debug)
            print_pair(lhs, rhs);
        while (reduce(rhs) or reduce(lhs))
            ;
        if (debug)
            std::cout << "EXPAND DIVISION START\n";
        if (debug)
            print_pair(lhs, rhs);
        expand_division(lhs, rhs);
        if (debug)
            print_pair(lhs, rhs);

        while (reduce(rhs) or reduce(lhs))
            ;
        if (debug)
            print_pair(lhs, rhs);
        if (debug)
            std::cout << "MULTIPLICATION EXPANSION START\n";
        while (expand_multiplication(lhs))
            ;
        while (expand_multiplication(rhs))
            ;
        if (debug)
            print_pair(lhs, rhs);

        while (reduce(rhs) or reduce(lhs))
            ;
        if (debug)
            print_pair(lhs, rhs);
        if (debug)
            std::cout << "MOVE NAMED TERMS START\n";

        while (every_term_contains_name(lhs, name) and every_term_contains_name(rhs, name)) {
            factor_remove(lhs, name);
            factor_remove(rhs, name);
            if (debug)
                print_pair(lhs, rhs);
        }

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

        lhs = new_lhs.value != node_type(op::EQUAL) ? new_lhs : math_tree_node(0);
        rhs = new_rhs.value != node_type(op::EQUAL) ? new_rhs : math_tree_node(0);
        if (debug)
            print_pair(lhs, rhs);

        if (debug)
            std::cout << "FACTORING START\n";

        factor(lhs, name);
        if (debug)
            print_pair(lhs, rhs);

        if (debug)
            std::cout << "FINAL ISOLATION START\n";

        if (lhs.value == node_type(op::MUL)) {
            rhs.emplace_left(rhs);
            rhs.emplace_right(lhs.get_right_child());
            rhs.value = op::DIV;
            lhs.value = lhs.get_left_child().value;
            lhs.children.first.reset();
            lhs.children.second.reset();
        }
        if (debug)
            print_pair(lhs, rhs);
        while (reduce(rhs) or reduce(lhs))
            ;

        print_pair(lhs, rhs);
        return rhs;
    }

    math_tree_node substitute(math_tree_node lhs, std::string name, const math_tree_node& with) {
        if (lhs.value == node_type(name)) {
            lhs.emplace_left(with.get_left_child());
            lhs.emplace_right(with.get_right_child());
            lhs.value = with.value;
            return lhs;
        }
        lhs.children.first = std::make_unique<math_tree_node>(substitute(lhs.get_left_child(), name, with));
        lhs.children.second = std::make_unique<math_tree_node>(substitute(lhs.get_right_child(), name, with));
        return lhs;
    }

    void in_place_substitute(math_tree_node& lhs, std::string name, const math_tree_node& with) {
        if (lhs.value == node_type(name)) {
            if (std::holds_alternative<op>(with.value)) {
                lhs.emplace_left(with.get_left_child());
                lhs.emplace_right(with.get_right_child());
            }
            lhs.value = with.value;
            return;
        }
        if (lhs.children.first)
            in_place_substitute(lhs.get_left_child(), name, with);
        if (lhs.children.second)
            in_place_substitute(lhs.get_right_child(), name, with);
        reduce(lhs);
    }
} // namespace ox::system
