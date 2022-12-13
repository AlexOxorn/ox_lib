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
#include <queue>
#include <ox/algorithms.h>
#include <ox/utils.h>

namespace ox {
    template <typename Node, typename GetNeighbours, typename Sentinel = Node,
              typename Heuristic = decltype([](...) { return 0; }), typename Hash = std::hash<Node>,
              typename DebugFunc = decltype([](const Node&) {})>
    requires requires(GetNeighbours g, Heuristic h, Node iter, Sentinel sent) {
                 { g(iter) } -> std::ranges::range;
                 { g(iter).begin()->first } -> std::convertible_to<Node>;
                 { g(iter).begin()->second } -> std::totally_ordered;
                 { h(iter, iter) } -> std::integral;
                 { h(iter, sent) } -> std::integral;
                 { iter == sent } -> std::convertible_to<bool>;
             }
    auto dikstra(Node start, Sentinel end, GetNeighbours get_neighbours_function, Heuristic heuristic_function = {},
                 Hash = Hash(), DebugFunc callback = DebugFunc())
            -> std::pair<std::vector<std::pair<Node, decltype(get_neighbours_function(start).begin()->second)>>,
                         decltype(get_neighbours_function(start).begin()->second)> {
        using cost_type =
                typename std::remove_reference<decltype(get_neighbours_function(start).begin()->second)>::type;
        using NodeCost = std::pair<Node, cost_type>;

        auto f_score_comp = [](const NodeCost& a, const NodeCost& b) {
            return a.second > b.second;
        };

        std::priority_queue open_set(f_score_comp, std::vector{std::make_pair(start, heuristic_function(start, end))});
        std::unordered_map<Node, Node, Hash> came_from;
        std::unordered_map<Node, cost_type, Hash> g_score;
        g_score[start] = 0;

        while (!open_set.empty()) {
            auto [current, current_cost] = open_set.top();
            open_set.pop();
            callback(current);
            if (current == end) {
                std::vector<std::pair<Node, cost_type>> to_return{std::make_pair(current, g_score.at(current))};
                auto came_from_iter = came_from.begin();
                while ((came_from_iter = came_from.find(to_return.back().first)) != came_from.end()) {
                    to_return.push_back(std::make_pair(came_from_iter->second, g_score.at(came_from_iter->second)));
                }
                stdr::reverse(to_return);
                return {to_return, g_score.at(current)};
            }

            for (auto [neighbour, cost] : get_neighbours_function(current)) {
                cost_type tentative_score = g_score.at(current) + cost;
                bool already_checked = g_score.contains(neighbour);
                if (!already_checked || tentative_score < g_score[neighbour]) {
                    came_from.insert_or_assign(neighbour, current);
                    g_score.insert_or_assign(neighbour, tentative_score);
                    cost_type f_cost = tentative_score + heuristic_function(current, end);
                    open_set.push(std::make_pair(neighbour, f_cost));
                }
            }
        }

        return {};
    }
} // namespace ox

#endif // OX_LIB__DIKSTRA_H
