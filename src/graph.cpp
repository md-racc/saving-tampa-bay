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

bool Graph::load(const std::string &nodePath, const std::string &edgePath) {}
