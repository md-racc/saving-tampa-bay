#pragma once

#include <iosfwd>
#include <string>

using namespace std;

void writeBenchmarkHeader(ostream& out);
void writeBenchmark(ostream& out, const string& algorithm,
                    bool found, double timeMs, int nodesExplored, double cost);
