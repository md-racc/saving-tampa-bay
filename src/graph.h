#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// adi - graph structs and adjacency list

// one dir
struct Edge {
  int to;
  double weight;
  double lenght{0};
  double risk{0};
};

struct Node {
  long long id;
  double lat;
  double lon;
};

class Graph {
public:
  void addNode(long long id, double lat, double lon);
  void addEdger(int from, int to, double length, double risk);
  bool load(const std::string &nodePath, const std::string &edgePath);
  int indexOf(long long id) const;

  const std::vector<std::vector<Edge>> &adjacent() const {
    return adj_;
  } // read
  const std::vector<Node> &nodes() const { return nodes_; }
  std::size_t size() const { return adj_.size(); }

  std::vector<std::vector<Edge>> &adj() { return adj_; } // write
  std::size_t edgeC() const;

private:
  std::vector<std::vector<Edge>> adj_;
  std::vector<Node> nodes_;
  std::unordered_map<long long, int> itToIndex_;
};

struct Poi {
  std::string name;
  std::string kind;
  int node;
};

bool loadPoi(const std::string &path, const Graph &g, std::vector<Poi> &out);

struct PathResult {
  bool found{false};
  double cost{0.0};
  int nodesExplored{0};
  std::vector<int> path;
};
