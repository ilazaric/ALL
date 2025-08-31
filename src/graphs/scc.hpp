#pragma once

#include <cassert>
#include <cstdint>
#include <ranges>
#include <vector>

#include <ivl/literals/ints.hpp>
using namespace ivl::literals::ints_exact;

#include "graph.hpp"

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

namespace ivl::graphs {

  // vec[x] == vec[y] iff x and y in same component
  // colors = 0..#
  std::vector<std::uint32_t> SCC(const DirectedGraph& G) {
    struct Engine {
      enum class Color { not_visited, on_stack, finished };

      struct NodeState {
        std::uint32_t time_seen;
        std::uint32_t time_done;
        std::uint32_t back_lo;
        std::uint32_t back_hi;
        Color         color = Color::not_visited;
      };

      const DirectedGraph&       G;
      std::vector<NodeState>     states;
      std::vector<std::uint32_t> stack;
      std::vector<std::uint32_t> partition;
      std::uint32_t              partition_count;
      std::uint32_t              time;

      Engine(const DirectedGraph& G)
          : G(G), states(G.node_count()), stack(), partition(G.node_count()), partition_count(0), time(0) {
        stack.reserve(G.node_count());
      }

      void process(std::uint32_t node) {
        states[node].time_seen = time;
        states[node].back_lo   = time;
        states[node].back_hi   = time;
        ++time;
        states[node].color = Color::on_stack;
        stack.push_back(node);

        for (auto next : G.outs(node)) {
          switch (states[next].color) {
          case Color::not_visited:
            process(next);
          case Color::on_stack:
            states[node].back_lo = std::min(states[node].back_lo, states[next].back_lo);
            states[node].back_hi = std::max(states[node].back_hi, states[next].back_hi);
          case Color::finished:
            break;
          }
        }

        states[node].time_done = time - 1;

        // found scc
        if (states[node].time_seen <= states[node].back_lo && states[node].time_done >= states[node].back_hi) {
          auto partition_index = partition_count++;
          while (true) {
            auto curr = stack.back();
            stack.pop_back();
            partition[curr]    = partition_index;
            states[curr].color = Color::finished;
            if (curr == node) break;
          }
        }
      }
    };

    Engine engine(G);

    for (auto node : std::views::iota(0_u32, G.node_count()))
      if (engine.states[node].color == Engine::Color::not_visited) engine.process(node);

    return engine.partition;
  }

} // namespace ivl::graphs
