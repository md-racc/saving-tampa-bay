#include "flood.h"

void applyFlood(Graph& graph, double severity) {
    for (auto& roads : graph.adj()) {
        for (auto& road : roads) {
            road.weight = road.length * (1.0 + severity * road.risk);
        }
    }
}
