#include <cctype>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include "astar.h"
#include "dijkstra.h"
#include "flood.h"
#include "graph.h"
#include "results.h"

using namespace std;

static const char *PLACES_PATH = "data/processed/places.csv";

static bool sameName(const string &a, const string &b) {
  if (a.size() != b.size()) {
    return false;
  }
  for (size_t i = 0; i < a.size(); ++i) {
    if (tolower(static_cast<unsigned char>(a[i])) !=
        tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

static int resolveNode(const string &arg, const Graph &graph,
                       const vector<Poi> &places) {
  size_t pos = 0;
  try {
    long long value = stoll(arg, &pos);
    if (pos == arg.size()) {
      if (value >= 1 && static_cast<size_t>(value) <= places.size()) {
        return places[value - 1].node;
      }
      return graph.indexOf(value);
    }
  } catch (const exception &) {
  }
  for (const Poi &place : places) {
    if (sameName(place.name, arg)) {
      return place.node;
    }
  }
  return -1;
}

int main(int argc, char **argv) {
  const string usage = "Usage: router <nodes.csv> <edges.csv> <start> <end> "
                       "<flood severity 0-1>\n"
                       "       router <nodes.csv> <edges.csv> --list\n"
                       "  <start>/<end>: a place number or name from --list "
                       "(quote names), or an OSM node ID\n";
  bool help = argc == 2 && string(argv[1]) == "--help";
  if (help) {
    cout << usage;
    return 0;
  }
  bool list = argc == 4 && string(argv[3]) == "--list";
  if (argc != 6 && !list) {
    cerr << usage;
    return 1;
  }

  double severity = 0;
  if (!list) {
    try {
      size_t pos;
      string flood(argv[5]);
      severity = stod(flood, &pos);
      if (pos != flood.size() || !isfinite(severity) || severity < 0 ||
          severity > 1) {
        throw invalid_argument("severity");
      }
    } catch (const exception &) {
      cerr << "Flood severity must be between 0 and 1.\n";
      return 1;
    }
  }
  Graph graph;
  if (!graph.loadCsv(argv[1], argv[2])) {
    cerr << "Graph data failed to load\n";
    return 1;
  }
  vector<Poi> places;
  bool havePlaces = loadPoi(PLACES_PATH, graph, places);
  if (list) {
    if (!havePlaces) {
      cerr << "Could not load " << PLACES_PATH << ".\n";
      return 1;
    }
    for (size_t i = 0; i < places.size(); ++i) {
      cout << setw(3) << i + 1 << ". [" << places[i].kind << "] "
           << places[i].name << '\n';
    }
    return 0;
  }
  int start = resolveNode(argv[3], graph, places);
  int end = resolveNode(argv[4], graph, places);
  if (start < 0 || end < 0) {
    cerr << "Start or end not found. Run with --list to see places.\n";
    return 1;
  }
  applyFlood(graph, severity);

  using Clock = chrono::steady_clock;
  auto begin = Clock::now();
  PathResult dijkstraResult = dijkstra(graph, start, end);
  auto middle = Clock::now();
  AStarSearch prepared(graph, start, end);
  auto searchBegin = Clock::now();
  PathResult astarResult = prepared.run();
  auto finish = Clock::now();
  double dijkstraMs = chrono::duration<double, milli>(middle - begin).count();
  double astarMs =
      chrono::duration<double, milli>(finish - searchBegin).count();
  double setupMs =
      chrono::duration<double, milli>(searchBegin - middle).count();
  cout << "A* heuristic setup: " << setupMs
       << " ms, setup + search: " << setupMs + astarMs << " ms\n";

  error_code error;
  filesystem::create_directories("output", error);
  if (error) {
    cerr << "Output directory failed to create: " << error.message() << '\n';
    return 1;
  }
  ofstream benchmarks("output/benchmarks.csv");
  ofstream routes("output/routes.csv");
  if (!benchmarks || !routes) {
    cerr << "The output files failed to open\n";
    return 1;
  }
  benchmarks << setprecision(17);
  writeBenchmarkHeader(benchmarks);
  writeRouteHeader(routes);
  const PathResult *results[] = {&dijkstraResult, &astarResult};
  string names[] = {"Dijkstra", "A*"};
  double times[] = {dijkstraMs, setupMs + astarMs};
  for (int i = 0; i < 2; ++i) {
    const PathResult &result = *results[i];
    cout << names[i] << ": ";
    if (result.found) {
      cout << "a route was found, cost " << result.cost;
    } else {
      cout << "there was no route found";
    }
    cout << ", total time " << times[i] << " ms, nodes explored "
         << result.nodesExplored << '\n';
    writeBenchmark(benchmarks, names[i], result.found, times[i],
                   result.nodesExplored, result.cost);
    if (result.found) {
      for (size_t j = 0; j < result.path.size(); ++j) {
        int index = result.path[j];
        if (index < 0 || static_cast<size_t>(index) >= graph.nodes().size()) {
          cerr << "The algorithm returned an invalid node index.\n";
          return 1;
        }
        const Node &node = graph.nodes()[index];
        writeRouteNode(routes, names[i], static_cast<int>(j), node.id, node.lat,
                       node.lon);
      }
    }
  }
  benchmarks.close();
  routes.close();
  if (!benchmarks || !routes) {
    cerr << "Failed to write output files\n";
    return 1;
  }
  cout << "The results saved to output/benchmarks.csv and output/routes.csv.\n";
  return 0;
}
