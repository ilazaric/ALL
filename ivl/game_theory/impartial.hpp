#pragma once

#include <ivl/ranges/input_range_over>
#include <boost/unordered/unordered_flat_map.hpp>
#include <concepts>
#include <ranges>
#include <type_traits>

namespace ivl::game_theory {
template<typename G>
concept impartial_game = requires(const G& g) {
  { g.moves() } -> ranges::input_range_over<G>;
};

template<impartial_game G>
struct grundifier {
  boost::unordered_flat_map<G, size_t> cache;

  size_t grundify(const G& g) {
    if (auto it = cache.find(g); it != cache.end()) {
      return it->second;
    }

    std::vector<size_t> moves_grundys;
    for (const G& child : g.moves()) moves_grundys.push_back(grundify(child));

    std::vector<char> mex(moves_grundys.size() + 1, 0);
    for (auto mg : moves_grundys)
      if (mg < mex.size()) mex[mg] = 1;

    for (size_t idx : std::views::iota(0ull, mex.size()))
      if (mex[idx] == 0) {
        cache.emplace(g, idx);
        return idx;
      }

    std::unreachable();
  }

  size_t operator()(const G& g) { return grundify(g); }
};
} // namespace ivl::game_theory
