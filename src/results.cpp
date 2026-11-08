#include "results.h"

#include <ostream>

using namespace std;

void writeBenchmarkHeader(ostream& out) {
    out << "algorithm,found,time_ms,nodes_explored,cost\n";
}

void writeBenchmark(ostream& out, const string& algorithm,
                    bool found, double timeMs, int nodesExplored, double cost) {
    out << algorithm << ',' << found << ',' << timeMs << ',' << nodesExplored << ',';
    if (found) {
        out << cost;
    }
    out << '\n';
}
