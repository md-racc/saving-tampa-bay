#include "astar.h"
#include "routing_search.h"

namespace {

double metersBetween(const Node& a, const Node& b) {
    constexpr double radians = 0.017453292519943295;
    constexpr double earthRadius = 6371000.0;
    double lat = std::sin((a.lat - b.lat) * radians / 2);
    double lon = std::sin((a.lon - b.lon) * radians / 2);
    double chord = lat * lat + std::cos(a.lat * radians) * std::cos(b.lat * radians) * lon * lon;
    return 2 * earthRadius * std::asin(std::sqrt(std::clamp(chord, 0.0, 1.0)));
}

} // namespace

AStarSearch::AStarSearch(const Graph& g, int src, int dst)
    : graph_(g), src_(src), dst_(dst) {
    if (!routing::validIndex(g, src) || !routing::validIndex(g, dst) || src == dst) return;

    const auto& nodes = g.nodes();
    const auto& edges = routing::neighbors(g, 0);
    remaining_.resize(g.size());
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        double meters = metersBetween(nodes[i], nodes[dst]);
        remaining_[i] = std::isfinite(meters) ? meters : 0.0;
    }

    // Keep the estimated savings along each road within its actual cost.
    // This also handles rounded lengths and graphs with different cost units.

    for (std::size_t from = 0; from < edges.size(); ++from) {
        for (const Edge& edge : edges[from]) {
            double gain = remaining_[from] - remaining_[edge.to];
            if (gain > 0 && edge.weight >= 0) {
                scale_ = std::min(scale_, edge.weight / gain);
            }
        }
    }
}

PathResult AStarSearch::run() const {
    // No estimates are needed for invalid endpoints or a zero-length route.
    return routing::search(graph_, src_, dst_, [this](int node) {
        return remaining_.empty() ? 0.0 : scale_ * remaining_[node];
    });
}

PathResult astar(const Graph& g, int src, int dst) {
    return AStarSearch(g, src, dst).run();
}
