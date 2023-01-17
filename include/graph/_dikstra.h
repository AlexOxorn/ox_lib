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

    template <typename Cmp>
    struct f_compare {
        Cmp cost_cmp;

        explicit f_compare(Cmp c) : cost_cmp(std::move(c)) {}

        bool operator()(const auto& a, const auto& b) { return cost_cmp(b.second, a.second); }
    };

    struct a_start {};
    struct heuristic_default {};

    template <
            // To represent a particular Node in a Graph
            typename Node,

            // Type representing the final Node in path searching
            typename Sentinel,

            // To represent the function to get heuristic measurements
            typename NeighbourFunction,

            // To represent the function to get heuristic measurements
            typename Heuristic = heuristic_default,

            // To represent the hashing function for storing the Nodes
            typename Hash = std::hash<Node>,

            // To compare relative costs
            typename CostComparison = std::less<>>
    class dikstra_solver {
    public:
        using NodeType = Node;
        using NeighbourRange = decltype(std::invoke(std::declval<NeighbourFunction>(), std::declval<Node>()));
        using NeighbourStep = NeighbourRange::value_type;
        using Cost = NeighbourStep::second_type;
        using NodeCost = std::pair<Node, Cost>;
        using ResultPath = std::vector<NodeCost>;
        using ResultType = std::pair<ResultPath, Cost>;

        using OpenSet = std::priority_queue<NodeCost, std::vector<NodeCost>, f_compare<CostComparison>>;
        using GScoreMap = std::unordered_map<Node, Cost, Hash>;
        using DirectionalMap = std::unordered_map<Node, Node, Hash>;
    private:
        using DebugFunc = std::function<void(const NodeType&, const Cost&, const OpenSet&, const GScoreMap&,
                                             const DirectionalMap&)>;
        constexpr static bool AStart = std::invocable<Heuristic, Node, Sentinel>;

        Node start;
        Sentinel end;
        NeighbourFunction get_neighbours;

        Heuristic _heuristic;

        CostComparison cmp;

        OpenSet open_set;
        DirectionalMap came_from;
        GScoreMap g_score;

        DebugFunc _debug_func;

        bool track_came_from = false;

        void track_path() {
            track_came_from = true;
        }

        Cost heuristic(const Node& a, const auto& b)
        requires AStart && std::convertible_to<Heuristic, bool>
        {
            return _heuristic ? std::invoke(_heuristic, a, b) : 0;
        }

        Cost heuristic(const Node& a, const Sentinel& b)
        requires AStart
        {
            return std::invoke(_heuristic, a, b);
        }

        template <typename... T>
        void debug(T&&... args) {
            if (_debug_func)
                _debug_func(std::forward<T>(args)...);
        }

        void process_neighbours(const Node& current) {
            for (auto [neighbour, cost] : std::invoke(get_neighbours, current)) {
                Cost tentative_score = g_score.at(current) + cost;
                bool already_checked = g_score.contains(neighbour);
                if (!already_checked || cmp(tentative_score, g_score[neighbour])) {
                    if (track_came_from)
                        came_from.insert_or_assign(neighbour, current);
                    g_score.insert_or_assign(neighbour, tentative_score);
                    Cost f_cost = tentative_score;
                    if constexpr (AStart)
                        f_cost += heuristic(neighbour, end);
                    open_set.push(std::make_pair(neighbour, f_cost));
                }
            }
        }
    public:
        dikstra_solver(Node _start, Sentinel end, NeighbourFunction get_neighbours, Hash = Hash(),
                       CostComparison cmp = CostComparison()) :
                start(std::move(_start)),
                end(std::move(end)),
                get_neighbours(std::move(get_neighbours)),
                cmp(cmp),
                open_set(f_compare(cmp), std::vector{std::make_pair(start, Cost{})}) {}

        dikstra_solver(a_start, Node _start, Sentinel end, NeighbourFunction get_neighbours,
                       Heuristic heuristic_function, Hash = Hash(), CostComparison cmp = CostComparison()) :
                start(std::move(_start)),
                end(std::move(end)),
                get_neighbours(std::move(get_neighbours)),
                _heuristic(std::move(heuristic_function)),
                cmp(cmp),
                open_set(f_compare(cmp), std::vector{std::make_pair(start, heuristic(start, end))}) {}

        void set_debug_func(DebugFunc&& f) { _debug_func = std::move(f); }

        ResultType generate_final_result(const Node& current) {
            ResultPath to_return{
                    {current, g_score.at(current)}
            };
            auto came_from_iter = came_from.begin();
            while ((came_from_iter = came_from.find(to_return.back().first)) != came_from.end()) {
                auto& node = came_from_iter->second;
                to_return.emplace_back(node, g_score.at(node));
            }
            stdr::reverse(to_return);
            return {to_return, g_score.at(current)};
        }

        ResultType operator()()
        requires requires(Node n, Sentinel s) {
                     { n == s } -> std::convertible_to<bool>;
                     { n == n } -> std::convertible_to<bool>;
                 }
        {
            g_score[start] = Cost{};

            while (!open_set.empty()) {
                auto [current, current_cost] = open_set.top();
                open_set.pop();
                debug(current, current_cost, open_set, g_score, came_from);
                if (current == end) {
                    return generate_final_result(current);
                }

                process_neighbours(current);
            }

            return {};
        }
    };

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
