#pragma once

namespace ivl::graphs {

  // TODO: generalize + concepts
  struct DirectedGraph {
    std::vector<std::vector<std::uint32_t>> edges;

    DirectedGraph(std::uint32_t n) : edges(n){}

    std::uint32_t node_count() const {
      return edges.size();
    }

    void add_edge(std::uint32_t a, std::uint32_t b){
      edges[a].push_back(b);
    }

    auto& outs(std::uint32_t a) const {
      return edges[a];
    }
  };

} // namespace ivl::graphs
