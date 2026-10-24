#pragma once

#include "collision_resolution"
#include <algorithm>
#include <vector>

namespace ivl::algorithm {
template<typename T>
void monotonic_vector_update_brute(std::vector<T>& a, const std::vector<T>& b, auto&& order, auto&& combine_or_erase) {
  auto ap = a.begin();
  auto bp = b.begin();
  std::vector<T> ret;
  while (ap != a.end() && bp != b.end()) {
    auto cmp = order(*ap, *bp);
    if (cmp < 0) {
      ret.push_back(*ap);
      ++ap;
      continue;
    }
    if (cmp > 0) {
      ret.push_back(*bp);
      ++bp;
      continue;
    }
    if (combine_or_erase(*ap, *bp) == monotonic_vector_update_collision_resolution::preserve) ret.push_back(*ap);
    ++ap;
    ++bp;
  }
  ret.insert(ret.end(), ap, a.end());
  ret.insert(ret.end(), bp, b.end());
  a = std::move(ret);
}
} // namespace ivl::algorithm
