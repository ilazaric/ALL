#pragma once

#include <algorithm>
#include <compare>
#include <utility>
#include <vector>

namespace ivl::algorithm {
// O(N log N) time
// O(log N) memory
template<typename It, typename Cmp = std::compare_three_way>
void monotonic_join_v5(It a, It b, It c, Cmp&& order = {}) {
  if (a == b || b == c) return;
  if (order(*(b - 1), *b) <= 0) return;
  while (order(*a, *b) <= 0) ++a;
  while (order(*(c - 1), *(b - 1)) >= 0) --c;

  auto half = (c - a) / 2;
  auto d = a;
  auto e = b;
  for (size_t i = 0; i < half; ++i) {
    if (d == b) ++e;
    else if (e == c) ++d;
    else if (order(*d, *e) <= 0) ++d;
    else ++e;
  }

  std::rotate(d, b, e);
  monotonic_join_v5(a, d, d + (e - b), order);
  monotonic_join_v5(e - (b - d), e, c, order);
}
} // namespace ivl::algorithm
