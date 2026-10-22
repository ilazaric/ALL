#pragma once

#include <algorithm>
#include <compare>
#include <utility>
#include <vector>

namespace ivl::algorithm {
// O(N log N) time
// O(log N) memory
template<typename It>
void monotonic_join_v5(It a, It b, It c) {
  if (a == b || b == c) return;
  if (*(b - 1) <= *b) return;
  while (*a <= *b) ++a;
  while (*(c - 1) >= *(b - 1)) --c;

  auto half = (c - a) / 2;
  auto d = a;
  auto e = b;
  for (size_t i = 0; i < half; ++i) {
    if (d == b) ++e;
    else if (e == c) ++d;
    else if (*d <= *e) ++d;
    else ++e;
  }

  std::rotate(d, b, e);
  monotonic_join_v5(a, d, d + (e - b));
  monotonic_join_v5(e - (b - d), e, c);
}
} // namespace ivl::algorithm
