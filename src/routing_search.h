#pragma once

#include "graph.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>
#include <tuple>
#include <vector>

namespace routing {

// The two graph headers expose the same edge list under different names.
template <class G>
auto neighbors(const G& graph, int) -> decltype(graph.adjacent()) {
    return graph.adjacent();
}

template <class G>
auto neighbors(const G& graph, long) -> decltype(graph.adj()) {
    return graph.adj();
}

inline bool validIndex(const Graph& graph, int index) {
    return index >= 0 && static_cast<std::size_t>(index) < graph.size();
}

template <class Estimate>
PathResult search(const Graph& graph, int src, int dst, Estimate estimate) {
    PathResult result;
    if (!validIndex(graph, src) || !validIndex(graph, dst)) return result;

    const auto& edges = neighbors(graph, 0);
    std::vector<double> distance(graph.size(), std::numeric_limits<double>::infinity());
    std::vector<int> parent(graph.size(), -1);
    using Entry = std::tuple<double, double, int>; // priority, cost so far, node
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pending;
    distance[src] = 0;
    pending.emplace(estimate(src), 0, src);

    while (!pending.empty()) {
        auto [priority, cost, node] = pending.top();
        pending.pop();
        if (cost != distance[node]) continue;
        ++result.nodesExplored;
        if (node == dst) {
            result.found = true;
            result.cost = cost;
            for (int at = dst; at != -1; at = parent[at]) result.path.push_back(at);
            std::reverse(result.path.begin(), result.path.end());
            break;
        }

        for (const Edge& edge : edges[node]) {
            if (edge.weight < 0 || !std::isfinite(edge.weight)) continue;
            double next = cost + edge.weight;
            if (next < distance[edge.to]) {
                distance[edge.to] = next;
                parent[edge.to] = node;
                pending.emplace(next + estimate(edge.to), next, edge.to);
            }
        }
    }
    return result;
}

} // namespace routing
