#pragma once

#include "graph.h"

// Prepare after flood weights are applied. Keep the graph unchanged until run().
class AStarSearch {
public:
    AStarSearch(const Graph& graph, int src, int dst);
    PathResult run() const;

private:
    const Graph& graph_;
    int src_;
    int dst_;
    std::vector<double> remaining_;
    double scale_ = 1.0;
};

PathResult astar(const Graph& g, int src, int dst);
