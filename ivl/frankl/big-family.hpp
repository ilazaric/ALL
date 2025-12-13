#pragma once

#include <concepts>
#include <cstddef>
#include <random>
#include <ranges>
#include <set>
#include <type_traits>
#include <utility>

#include <boost/dynamic_bitset.hpp>

using BigBS = boost::dynamic_bitset<>;

struct BigFamily {
  std::set<BigBS> family;

  template <typename Generator>
  static BigBS random_bs(Generator& g, std::uint32_t atomcount, double elemprob) {
    BigBS                       out(atomcount);
    std::bernoulli_distribution dist(elemprob);
    for (std::uint32_t i = 0; i < atomcount; ++i)
      if (dist(g)) out.set(i);
    return out;
  }

  template <typename Generator>
  static BigFamily random(Generator& g, std::uint32_t atomcount, std::uint32_t basiscount, double elemprob) {
    BigFamily bf;
    bf.family.emplace(atomcount);
    for (std::uint32_t rep = 0; rep < basiscount; ++rep) {
      auto basiselem = random_bs(g, atomcount, elemprob);
      if (basiselem.none()) continue;
      for (auto& bs : bf.family | std::views::reverse) {
        auto tmp = bs | basiselem;
        if (tmp == bs) continue;
        bf.family.insert(std::move(tmp));
      }
    }
    return bf;
  }
};
