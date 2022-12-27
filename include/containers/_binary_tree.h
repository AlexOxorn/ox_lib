//
// Created by alexoxorn on 2021-12-20.
//

#ifndef ADVENTOFCODE__BINARY_TREE_H
#define ADVENTOFCODE__BINARY_TREE_H

#include <vector>
#include <optional>
#include <memory>
#include <stack>
#include <exception>
#include <utility>

namespace ox {
    template<typename Value>
    class binary_tree;

    template<typename Value>
    class binary_tree_node {
    public:
        Value value;

    protected:
        binary_tree_node* parent = nullptr;

    public:
        std::pair<std::unique_ptr<binary_tree_node>, std::unique_ptr<binary_tree_node>> children;

        binary_tree_node(binary_tree_node&& b) noexcept : value(std::move(b.value)), parent(b.parent), children(std::move(b.children)) {
            if (children.first)
                children.first->parent = this;
            if (children.second)
                children.second->parent = this;
        }
        binary_tree_node& operator=(binary_tree_node&& b) noexcept {
            value = std::move(b.value);
            parent = b.parent;
            children = std::move(b.children);
            if (children.first)
                children.first->parent = this;
            if (children.second)
                children.second->parent = this;
            return *this;
        }

        binary_tree_node(const binary_tree_node& b) noexcept : value(b.value), parent(nullptr) {
            if (b.children.first)
                emplace_left(b.get_left_child());
            if (b.children.second)
                emplace_right(b.get_right_child());
        }
        binary_tree_node& operator=(const binary_tree_node& b) {
            if (this == &b)
                return *this;
            value = b.value;
            parent = nullptr;
            if (b.children.first)
                emplace_left(b.get_left_child());
            if (b.children.second)
                emplace_right(b.get_right_child());
            return *this;
        }

        template<typename... Args>
        requires std::constructible_from<Value, Args...>
        explicit binary_tree_node(Args... args)
            : value(args...),
              parent(nullptr) {}

        template<typename>
        friend class binary_tree;

        template<typename... Args>
        auto& emplace_left(Args&&... args) {
            children.first = std::make_unique<ox::binary_tree_node<Value>>(std::forward<Args>(args)...);
            children.first->parent = this;
            return *children.first;
        }

        template<typename... Args>
        auto& emplace_right(Args&&... args) {
            children.second = std::make_unique<ox::binary_tree_node<Value>>(std::forward<Args>(args)...);
            children.second->parent = this;
            return *children.second;
        }

        auto& emplace_left(binary_tree<Value>&& tree) {
            return emplace_left(std::move(tree.head));
        }

        auto& emplace_right(binary_tree<Value>&& tree) {
            return emplace_right(std::move(tree.head));
        }

        [[nodiscard]] int get_depth() const {
            if (parent == nullptr) {
                return 0;
            }
            return 1 + parent->get_depth();
        }

        binary_tree_node& get_left_child() { return *children.first; }
        binary_tree_node& get_right_child() { return *children.second; }
        const binary_tree_node& get_left_child() const { return *children.first; }
        const binary_tree_node& get_right_child() const { return *children.second; }
        [[nodiscard]] bool has_parent() const {
            return parent != nullptr;
        }
        binary_tree_node& get_parent() { return *parent; }
        const binary_tree_node& get_parent() const { return *parent; }
    };

    template<typename Value>
    class binary_tree {
        template<typename> friend class binary_tree_node;
    public:
        class iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using node = binary_tree_node<Value>;

        using value_type = Value;
        binary_tree_node<value_type> head;

        explicit binary_tree(const Value& v) : head(v) {};
        explicit binary_tree(binary_tree_node<Value>&& v) : head(std::forward<binary_tree_node<Value>&&>(v)) {};
        binary_tree() = default;

        iterator begin() {
            return iterator(head);
        }
        iterator end() {
            return iterator(head, true);
        }

        reverse_iterator rbegin() {
            return reverse_iterator(end());
        }
        reverse_iterator rend() {
            return reverse_iterator(begin());
        }
    };


    template<typename Value>
    class binary_tree<Value>::iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = binary_tree_node<Value>;
        using difference_type = int;
        using pointer = value_type*;
        using referece = value_type&;
        using const_pointer = const value_type*;
        using const_reference = const value_type&;
    private:
        using self = binary_tree<Value>::iterator;
        binary_tree_node<Value>* current_node;
        bool end = false;

        static referece get_leftmost(pointer current) {
            while (current->children.first) {
                current = current->children.first.get();
            }
            return *current;
        }

        static referece get_rightmost(pointer current) {
            while (current->children.second) {
                current = current->children.second.get();
            }
            return *current;
        }
    public:
        iterator() : current_node(nullptr), end(true) {}

        explicit iterator(binary_tree_node<Value>& source, bool _end = false) :
              current_node(_end ? &get_rightmost(&source) : &get_leftmost(&source)),
              end(_end) {};

        referece operator *() const {
            return *current_node;
        }

        pointer operator ->() const {
            return current_node;
        }

        self& operator++() {
            if (current_node && end) {
                end = false;
                return *this;
            }
            if (current_node->children.second) {
                current_node = &get_leftmost(current_node->children.second.get());
            } else {
                auto original_current = current_node;
                while (current_node->parent != nullptr && current_node->parent->children.second.get() == current_node) {
                    current_node = current_node->parent;
                }
                if (current_node->parent == nullptr) {
                    current_node = original_current;
                    end = true;
                } else {
                    current_node = current_node->parent;
                }
            }
            return *this;
        }

        self operator++(int) {
            auto cpy(*this);
            ++(*this);
            return cpy;
        }

        self& operator--() {
            if (current_node && end) {
                end = false;
                return *this;
            }
            if (current_node->children.first) {
                current_node = &get_rightmost(current_node->children.first.get());
            } else {
                auto original_current = current_node;
                while (current_node->parent != nullptr && current_node->parent->children.first.get() == current_node) {
                    current_node = current_node->parent;
                }
                if (current_node->parent == nullptr) {
                    current_node = original_current;
                    end = true;
                } else {
                    current_node = current_node->parent;
                }
            }
            return *this;
        }

        self operator--(int) {
            auto cpy(*this);
            --(*this);
            return cpy;
        }

        auto operator<=>(const self& other) const {
            if (other.end && !end) {
                return std::partial_ordering::less;
            }
            return std::partial_ordering::unordered;
        }

        auto operator==(const self& other) const {
            if (other.end && end)
                return true;
            if (other.end != end)
                return false;
            return this->current_node == other.current_node;
        }
    };
} // namespace ox

#endif // ADVENTOFCODE__BINARY_TREE_H
