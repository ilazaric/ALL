#pragma once

#include <algorithm>
#include <compare>
#include <utility>
#include <vector>

namespace ivl::algorithm {
template<typename It>
void monotonic_join_v6(It a, It b, It c) {
  if (a == b || b == c) return;
  if (*(b - 1) <= *b) return;
  while (*a <= *b) ++a;
  while (*(c - 1) >= *(b - 1)) --c;

  std::iter_swap(a, b);
  It ap = a + 1;
  It bp = b + 1;
  It mp = b;
  It ep = b + 1;
  size_t rem = 1;
  while (ap != b && bp != c) {
    if (*bp <= *mp) {
      std::iter_swap(bp, ap);
      if (ep == bp && mp == b) ++ep, ++rem;
      ++bp;
      ++ap;
      continue;
    }

    std::iter_swap(ap, mp);
    ++ap;
    ++mp;
    --rem;
    if (mp == ep) mp = b;
    if (rem == 0) {
      if (ep != bp) {
        std::rotate(b, mp, ep);
        monotonic_join_v6(b, ep, bp);
        mp = b;
        ep = bp;
      }
      rem = bp - b;
    }
  }

  while (ap != b) {
    std::iter_swap(ap, mp);
    ++ap;
    ++mp;
    --rem;
    if (mp == ep) mp = b;
    if (rem == 0) {
      if (ep != bp) {
        std::rotate(b, mp, ep);
        monotonic_join_v6(b, ep, bp);
        mp = b;
        ep = bp;
      }
      rem = bp - b;
    }
  }

  std::rotate(b, mp, ep);
  monotonic_join_v6(b + rem, ep, bp);
  monotonic_join_v6(b, bp, c);
}
} // namespace ivl::algorithm
