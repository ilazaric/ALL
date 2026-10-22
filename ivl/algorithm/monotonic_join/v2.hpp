#pragma once

#include <algorithm>
#include <compare>
#include <utility>
#include <vector>

namespace ivl::algorithm {
// O(N) time
// O(N) memory
template<typename It, typename Cmp = std::compare_three_way>
void monotonic_join_v2(It a, It b, It c, Cmp&& order = {}) {
  if (a == b || b == c) return;
  std::vector<size_t> perm(c - a);
  {
    size_t i = 0;
    It p = a;
    It q = b;
    while (p != b && q != c) {
      if (order(*p, *q) <= 0) perm[i++] = p++ - a;
      else perm[i++] = q++ - a;
    }
    while (p != b) perm[i++] = p++ - a;
    while (q != c) perm[i++] = q++ - a;
  }
  size_t dead = -1;
  for (size_t i = 0; i < perm.size(); ++i) {
    if (perm[i] <= i || perm[i] == dead) continue;
    for (size_t j = i; perm[j] != i; j = std::exchange(perm[j], dead)) std::iter_swap(a + j, a + perm[j]);
  }
}
} // namespace ivl::algorithm
