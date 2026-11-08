#include "dijkstra.h"
#include "routing_search.h"

PathResult dijkstra(const Graph& g, int src, int dst) {
    return routing::search(g, src, dst, [](int) { return 0.0; });
}
