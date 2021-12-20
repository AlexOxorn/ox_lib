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

namespace ox {
    template<typename Node, typename GetNeighbours, typename Distance = std::minus<>, typename Hash = std::hash<Node>>
    requires requires (GetNeighbours g, Distance d, Node iter) {
        { g(iter) } -> std::ranges::range;
        { *(g(iter).begin()) } -> std::convertible_to<Node>;
        { d(iter, iter) } -> std::integral;
    }
    std::vector<Node>
    dikstra(Node start, Node end, GetNeighbours get_neighbours_function, Distance distance_function = {}, Hash hashing_function = {}) {
        std::vector<Node> open_set{start};
        std::unordered_set<Node, Hash> closed_set;
        std::unordered_map<Node, Node, Hash> came_from;
        std::unordered_map<Node, long, Hash> g_score;
        g_score[start] = 0;

        auto g_score_comp = [&g_score] (const Node& a, const Node& b) {
            return g_score[a] > g_score[b];
        };

        while (!open_set.empty()) {
            Node current = ox::pop_heap_value(open_set, g_score_comp);
            closed_set.insert(current);
            if (current == end) {
                std::vector<Node> to_return {end};
                Node curr = end;
                auto came_from_iter = came_from.begin();
                while ((came_from_iter = came_from.find(to_return.back())) != came_from.end()) {
                    to_return.push_back(came_from_iter->second);
                }
                stdr::reverse(to_return);
                return to_return;
            }

            g_score.try_emplace(current, std::numeric_limits<long>::max());
            for (Node neighbour : get_neighbours_function(current)) {
                g_score.try_emplace(neighbour, std::numeric_limits<long>::max());
                int tentative_score = g_score[current] + distance_function(neighbour, current);
                if (tentative_score < g_score[neighbour]) {
                    came_from.insert_or_assign(neighbour, current);
                    g_score.insert_or_assign(neighbour, tentative_score);
                }
                bool not_in_open = std::ranges::find(open_set, neighbour) == open_set.end();
                if (!closed_set.contains(neighbour)) {
                    if (not_in_open) {
                        ox::push_heap_value(open_set, neighbour, g_score_comp);
                    } else {
                        auto not_heap = std::ranges::is_heap_until(open_set, g_score_comp);
                        if (not_heap != open_set.end())
                            std::push_heap(open_set.begin(), not_heap + 1, g_score_comp);
                    }
                }
            }
        }

        return {};
    }
}

#endif //OX_LIB__DIKSTRA_H
