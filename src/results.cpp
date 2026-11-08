#include "results.h"

#include <iomanip>
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

void writeRouteHeader(ostream& out) {
    out << "algorithm,sequence,node_id,lat,lon\n";
}

void writeRouteNode(ostream& out, const string& algorithm, int sequence,
                    long long nodeId, double lat, double lon) {
    out << setprecision(17) << algorithm << ',' << sequence << ',' << nodeId << ',' << lat << ',' << lon << '\n';
}
