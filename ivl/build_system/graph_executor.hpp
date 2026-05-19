#pragma once

#include <map>
#include <string>
#include <vector>
#include <set>

namespace ivl::build_system {

// TODO: internal nodes can be uint32_t
  
template <typename T>
struct graph {
  std::vector<T> external_nodes;
  std::map<T, size_t> node_encoder;

  std::vector<std::vector<size_t>> edges;
  std::vector<std::vector<size_t>> back_edges;

  // size_t encode(const T& a) {
  //   auto it = node_encoder.find(a);
  //   if (it != node_encoder.end()) return it->second;
  //   node_encoder[a] = external_nodes.size();
  //   external_nodes.push_back(a);
  //   return external_nodes.size() - 1;
  // }

  // void add_edge_internal(size_t a, size_t b) {
  //   edges[a].insert(b);
  //   back_edges[b].insert(a);
  // }

  // void add_edge(const T& a, const T& b) { add_edge_internal(encode(a), encode(b)); }
};
} // namespace ivl::build_system
