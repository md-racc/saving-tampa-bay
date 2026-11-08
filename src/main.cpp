#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <string>

#include "astar.h"
#include "dijkstra.h"
#include "flood.h"
#include "graph.h"
#include "results.h"

using namespace std;

int main(int argc, char** argv) {
    const string usage = "Usage: router <nodes.csv> <edges.csv> <start node ID> <end node ID> <flood severity 0-1>\n";
    bool help = argc == 2 && string(argv[1]) == "--help";
    if (help) {
        cout << usage;
        return 0;
    }
    if (argc != 6) {
        cerr << usage;
        return 1;
    }

    long long startId, endId;
    double severity;
    try {
        size_t pos;
        string start(argv[3]), end(argv[4]), flood(argv[5]);
        startId = stoll(start, &pos);
        if (pos != start.size()) throw invalid_argument("start");
        endId = stoll(end, &pos);
        if (pos != end.size()) throw invalid_argument("end");
        severity = stod(flood, &pos);
        if (pos != flood.size() || !isfinite(severity) || severity < 0 || severity > 1)
            throw invalid_argument("severity");
    } catch (const exception&) {
        cerr << "Node IDs must be integers and flood severity must be between 0 and 1.\n";
        return 1;
    }

    Graph graph;
    if (!graph.loadCsv(argv[1], argv[2])) {
        cerr << "Could not load graph data.\n";
        return 1;
    }
    int start = graph.indexOf(startId);
    int end = graph.indexOf(endId);
    if (start < 0 || end < 0) {
        cerr << "Start or end node ID was not found.\n";
        return 1;
    }
    applyFlood(graph, severity);

    using Clock = chrono::steady_clock;
    auto begin = Clock::now();
    PathResult dijkstraResult = dijkstra(graph, start, end);
    auto middle = Clock::now();
    PathResult astarResult = astar(graph, start, end);
    auto finish = Clock::now();
    double dijkstraMs = chrono::duration<double, milli>(middle - begin).count();
    double astarMs = chrono::duration<double, milli>(finish - middle).count();

    error_code error;
    filesystem::create_directories("output", error);
    if (error) {
        cerr << "Could not create output directory: " << error.message() << '\n';
        return 1;
    }
    ofstream benchmarks("output/benchmarks.csv"), routes("output/routes.csv");
    if (!benchmarks || !routes) {
        cerr << "Could not open output files.\n";
        return 1;
    }
    benchmarks << setprecision(17);
    writeBenchmarkHeader(benchmarks);
    writeRouteHeader(routes);
    const PathResult* results[] = {&dijkstraResult, &astarResult};
    string names[] = {"Dijkstra", "A*"};
    double times[] = {dijkstraMs, astarMs};
    for (int i = 0; i < 2; ++i) {
        const PathResult& result = *results[i];
        cout << names[i] << ": ";
        if (result.found) cout << "route found, cost " << result.cost;
        else cout << "no route found";
        cout << ", time " << times[i] << " ms, nodes explored " << result.nodesExplored << '\n';
        writeBenchmark(benchmarks, names[i], result.found, times[i], result.nodesExplored, result.cost);
        if (result.found) {
            for (size_t j = 0; j < result.path.size(); ++j) {
                int index = result.path[j];
                if (index < 0 || static_cast<size_t>(index) >= graph.nodes().size()) {
                    cerr << "Algorithm returned an invalid node index.\n";
                    return 1;
                }
                const Node& node = graph.nodes()[index];
                writeRouteNode(routes, names[i], static_cast<int>(j), node.id, node.lat, node.lon);
            }
        }
    }
    benchmarks.close();
    routes.close();
    if (!benchmarks || !routes) {
        cerr << "Could not write output files.\n";
        return 1;
    }
    cout << "Results saved to output/benchmarks.csv and output/routes.csv.\n";
    return 0;
}
