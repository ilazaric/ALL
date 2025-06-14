#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>

#include <boost/unordered/unordered_flat_map.hpp>

namespace ivl::gt {

  template <typename T, typename U>
  concept range_of = std::ranges::range<T> && std::same_as<U, std::ranges::range_value_t<T>>;

  template <typename G>
  concept ImpartialGame = requires(const G& g) {
    { g.moves() } -> range_of<G>;
  };

  template <ImpartialGame G>
  struct Grundifier {
    boost::unordered_flat_map<G, std::size_t> cache;

    std::size_t grundify(const G& g) {
      if (auto it = cache.find(g); it != cache.end()) {
        return it->second;
      }

      std::vector<std::size_t> moves_grundys;
      for (const G& child : g.moves())
        moves_grundys.push_back(grundify(child));

      std::vector<char> mex(moves_grundys.size() + 1, 0);
      for (auto mg : moves_grundys)
        if (mg < mex.size())
          mex[mg] = 1;

      for (std::size_t idx : std::views::iota(0ull, mex.size()))
        if (mex[idx] == 0) {
          cache.emplace(g, idx);
          return idx;
        }

      // std::unreachable();
      throw;
    }

    std::size_t operator()(const G& g) { return grundify(g); }
  };

} // namespace ivl::gt
