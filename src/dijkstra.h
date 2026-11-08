#pragma once

#include "graph.h"

// Shortest path from src to dst (from scratch; min-priority queue).
PathResult dijkstra(const Graph& g, int src, int dst);
