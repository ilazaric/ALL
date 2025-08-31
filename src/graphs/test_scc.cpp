#include <iostream>
#include <random>
#include <ranges>

#include "scc.hpp"

#include <ivl/io/stlutils.hpp>

std::mt19937 gen(101);

void dfs(std::uint32_t node, const ivl::graphs::DirectedGraph& G, std::vector<bool>& reach) {
  if (reach[node]) return;

  reach[node] = true;
  for (auto next : G.outs(node))
    dfs(next, G, reach);
}

void run_test() {
  constexpr std::uint32_t                      node_count = 3000;
  constexpr std::uint32_t                      edge_count = 10000;
  ivl::graphs::DirectedGraph                   G(node_count);
  std::uniform_int_distribution<std::uint32_t> dist(0_u32, node_count - 1);
  for (auto ti : std::views::iota(0_u32, edge_count)) {
    auto a = dist(gen);
    auto b = dist(gen);
    G.add_edge(a, b);
  }

  auto partition = SCC(G); // thanks ADL

  // LOG(std::ranges::max(partition));

  auto reach = std::vector(node_count, std::vector(node_count, false));

  for (auto i : std::views::iota(0_u32, node_count))
    dfs(i, G, reach[i]);

  for (auto i : std::views::iota(0_u32, node_count))
    for (auto j : std::views::iota(0_u32, node_count))
      if ((reach[i][j] && reach[j][i]) != (partition[i] == partition[j])) throw std::runtime_error("test failed");
}

void test1() {
  ivl::graphs::DirectedGraph G(4);
  G.add_edge(0, 3);
  G.add_edge(0, 1);
  G.add_edge(1, 2);
  G.add_edge(2, 3);
  G.add_edge(3, 2);
  G.add_edge(3, 0);
  auto res = SCC(G);
  LOG(res);
  exit(0);
}

int main() {
  // test1();

  while (true) {
    run_test();
    std::cout << '.' << std::flush;
  }
}
