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

PathResult astar(const Graph& g, int src, int dst) {
    if (!routing::validIndex(g, src) || !routing::validIndex(g, dst)) return {};
    if (src == dst) return routing::search(g, src, dst, [](int) { return 0.0; });

    const auto& nodes = g.nodes();
    const auto& edges = routing::neighbors(g, 0);
    std::vector<double> remaining(g.size());
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        double meters = metersBetween(nodes[i], nodes[dst]);
        remaining[i] = std::isfinite(meters) ? meters : 0.0;
    }

    // Keep the estimated savings along each road within its actual cost.
    // This also handles rounded lengths and graphs with different cost units.
    double scale = 1.0;
    for (std::size_t from = 0; from < edges.size(); ++from) {
        for (const Edge& edge : edges[from]) {
            double gain = remaining[from] - remaining[edge.to];
            if (gain > 0 && edge.weight >= 0) {
                scale = std::min(scale, edge.weight / gain);
            }
        }
    }
    return routing::search(g, src, dst, [&](int node) { return scale * remaining[node]; });
}
