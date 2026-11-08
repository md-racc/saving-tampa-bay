#include <iostream>
#include <cmath>
#include <stdexcept>
#include <string>

using namespace std;

int main(int argc, char** argv) {
    bool help = argc == 2 && string(argv[1]) == "--help";
    if (help) {
        cout << "Usage: router <nodes.csv> <edges.csv> <start node ID> <end node ID> <flood severity 0-1>\n";
        return 0;
    }
    if (argc != 6) {
        cerr << "Usage: router <nodes.csv> <edges.csv> <start node ID> <end node ID> <flood severity 0-1>\n";
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

    (void)startId;
    (void)endId;
    cerr << "Graph loading and routing are not implemented yet.\n";
    return 1;
}
