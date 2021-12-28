//
// Created by alexoxorn on 2021-12-18.
//

#ifndef OX_LIB__DIKSTRA_H
#define OX_LIB__DIKSTRA_H

#include <concepts>
#include <ranges>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <ox/algorithms.h>
#include <ox/utils.h>

namespace ox {
    template<typename Node, typename GetNeighbours, typename Heuristic = decltype([](...) { return 0; }), typename Hash = std::hash<Node>>
    requires requires (GetNeighbours g, Heuristic h, Node iter) {
        { g(iter) } -> std::ranges::range;
        { g(iter).begin()->first } -> std::convertible_to<Node>;
        { g(iter).begin()->second } -> remove_reference_integral;
        { h(iter, iter) } -> std::integral;
    }
    auto dikstra(Node start, Node end, GetNeighbours get_neighbours_function, Heuristic heuristic_function = {}, Hash = {}) ->
    std::pair<std::vector<Node>, decltype(get_neighbours_function(start).begin()->second)> {
        std::vector<Node> open_set{start};
        std::unordered_set<Node, Hash> closed_set;
        std::unordered_map<Node, Node, Hash> came_from;
        std::unordered_map<Node, long, Hash> g_score;
        std::unordered_map<Node, long, Hash> f_score;
        g_score[start] = 0;
        f_score[start] = heuristic_function(start, end);

        auto f_score_comp = [&f_score] (const Node& a, const Node& b) {
            return f_score[a] > f_score[b];
        };

        while (!open_set.empty()) {
            Node current = ox::pop_heap_value(open_set, f_score_comp);
            closed_set.insert(current);
            if (current == end) {
                std::vector<Node> to_return {end};
                auto came_from_iter = came_from.begin();
                while ((came_from_iter = came_from.find(to_return.back())) != came_from.end()) {
                    to_return.push_back(came_from_iter->second);
                }
                stdr::reverse(to_return);
                return {to_return, g_score.at(current)};
            }

            g_score.try_emplace(current, std::numeric_limits<long>::max());
            for (auto [neighbour, cost] : get_neighbours_function(current)) {
                g_score.try_emplace(neighbour, std::numeric_limits<long>::max());
                int tentative_score = g_score[current] + cost;
                if (tentative_score < g_score[neighbour]) {
                    came_from.insert_or_assign(neighbour, current);
                    g_score.insert_or_assign(neighbour, tentative_score);
                    f_score.insert_or_assign(neighbour, tentative_score + heuristic_function(current, end));
                }
                bool not_in_open = std::ranges::find(open_set, neighbour) == open_set.end();
                if (!closed_set.contains(neighbour)) {
                    if (not_in_open) {
                        ox::push_heap_value(open_set, neighbour, f_score_comp);
                    } else {
                        auto not_heap = std::ranges::is_heap_until(open_set, f_score_comp);
                        if (not_heap != open_set.end())
                            std::push_heap(open_set.begin(), not_heap + 1, f_score_comp);
                    }
                }
            }
        }

        return {};
    }
}

#endif //OX_LIB__DIKSTRA_H
