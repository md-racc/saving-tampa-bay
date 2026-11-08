#include "graph.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

// adi - load the road data

// CSV helpers

std::vector<std::string> splitCSV(const std::string &line) {
  std::vector<std::string> fields;
  std::stringstream ss(line);
  std::string field;
  while (std::getline(ss, field, ',')) {
    fields.push_back(field);
  }
  return fields;
}

bool readLine(std::istream &in, std::string &line, int &lineN) {
  if (!std::getline(in, line))
    return false;
  ++lineN;
  if (!line.empty() && line.back() == '\r')
    line.pop_back();
  return true;
}

bool fail(const std::string &path, int lineN, const std::string &msg) {
  std::cerr << path << ":" << lineN << ": " << msg << '\n';
  return false;
}

// build
void Graph::addNode(long long id, double lat, double lon) {
  idToIndex_[id] = static_cast<int>(nodes_.size());
  nodes_.push_back(Node{id, lat, lon});
  adj_.emplace_back();
}

void Graph::addEdge(int from, int to, double length, double risk) {
  adj_[from].push_back(Edge{to, length, length, risk});
}

int Graph::indexOf(long long id) const {
  auto it = idToIndex_.find(id);
  return it == idToIndex_.end() ? -1 : it->second;
}

std::size_t Graph::edgeC() const {
  std::size_t total = 0;
  for (const auto &out : adj_)
    total += out.size();
  return total;
}

// CSV LOADER
bool Graph::loadCsv(const std::string &nodesPath, const std::string &edgesPath) {
  adj_.clear();
  nodes_.clear();
  idToIndex_.clear();

  std::ifstream nodesIn(nodesPath);
  if (!nodesIn.is_open())
    return fail(nodesPath, 0, "could not open nodes file");

  std::string line;
  int lineNo = 0;
  readLine(nodesIn, line, lineNo); // skip header

  while (readLine(nodesIn, line, lineNo)) {
    if (line == "")
      continue;

    std::vector<std::string> f = splitCSV(line);
    if (f.size() < 3)
      return fail(nodesPath, lineNo, "bad line");

    long long id;
    double lat, lon;
    try {
      id = std::stoll(f[0]);
      lat = std::stod(f[1]);
      lon = std::stod(f[2]);
    } catch (...) {
      return fail(nodesPath, lineNo, "bad number");
    }

    if (idToIndex_.count(id) > 0)
      return fail(nodesPath, lineNo, "duplicate id");

    addNode(id, lat, lon);
  }

  std::ifstream edgesIn(edgesPath);
  if (!edgesIn.is_open())
    return fail(edgesPath, 0, "could not open edges file");

  lineNo = 0;
  readLine(edgesIn, line, lineNo); // skip header

  while (readLine(edgesIn, line, lineNo)) {
    if (line == "")
      continue;

    std::vector<std::string> f = splitCSV(line);
    if (f.size() < 4)
      return fail(edgesPath, lineNo, "bad line");

    try {
      long long fromId = std::stoll(f[0]);
      long long toId = std::stoll(f[1]);
      double length = std::stod(f[2]);
      double risk = std::stod(f[3]);

      int from = indexOf(fromId);
      int to = indexOf(toId);
      if (from == -1 || to == -1)
        return fail(edgesPath, lineNo, "unknown node");

      if (length < 0 || risk < 0 || risk > 1)
        return fail(edgesPath, lineNo, "invalid length or risk");

      addEdge(from, to, length, risk);
    } catch (...) {
      return fail(edgesPath, lineNo, "bad number");
    }
  }

  return true;
}

bool loadPoi(const std::string &path, const Graph &g, std::vector<Poi> &out) {
  out.clear();

  std::ifstream in(path);
  if (!in)
    return fail(path, 0, "cannot open file");

  std::string line;
  int lineNo = 0;
  if (!readLine(in, line, lineNo))
    return fail(path, 0, "file is empty");
  // kip header ("name,kind,id").

  while (readLine(in, line, lineNo)) {
    if (line.empty())
      continue;

    std::vector<std::string> f = splitCSV(line);
    if (f.size() < 3)
      return fail(path, lineNo, "expected name,kind,id");

    long long id;
    try {
      id = std::stoll(f[2]);
    } catch (const std::exception &) {
      return fail(path, lineNo, "not a number in: " + line);
    }

    int node = g.indexOf(id);
    if (node < 0)
      return fail(path, lineNo, "place uses a node id missing from nodes.csv");

    out.push_back(Poi{f[0], f[1], node});
  }
  return true;
}
