#pragma once

#include <ivl/algorithm/monotonic_vector_update>
#include "concepts"
#include <cstddef>
#include <vector>

template<size_t N>
struct monomial {
  std::array<size_t, N> orders;
};

template<typename K, size_t N>
struct coef_monomial {
  K coef;
  monomial<N> mono;
};

template<field K, size_t N, typename Order>
struct polynomial {
  std::vector<coef_monomial<K, N>> monos;
  [[no_unique_address]] Order order;

  polynomial& operator+=(const polynomial& b) {
    ivl::algorithm::monotonic_vector_update(monos, b.monos, order, [](auto& l, const auto& r) {
      if (!r) return;
      if (!l) {
        l.emplace(*r);
        return;
      }
      l->coef += r->coef;
      if (l->coef == 0) l.reset();
    });
  }

  friend polynomial operator+(polynomial a, const polynomial& b) {
    a += b;
    return a;
  }
};
