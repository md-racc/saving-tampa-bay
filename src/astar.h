#pragma once

#include "graph.h"

// A* shortest path using a straight-line (haversine) heuristic.
PathResult astar(const Graph& g, int src, int dst);
