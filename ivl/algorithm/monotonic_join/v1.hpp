#pragma once

#include <algorithm>
#include <compare>
#include <type_traits>
#include <utility>
#include <vector>

namespace ivl::algorithm {
// O(N) time
// O(N) memory
template<typename It, typename Cmp = std::compare_three_way>
void monotonic_join_v1(It a, It b, It c, Cmp&& order = {}) {
  while (a != b && b != c) {
    std::vector<std::remove_cvref_t<decltype(*a)>> vec;
    size_t vec_end = 0;
    It p = b;
    for (; a != b && (p != c || vec.size() != vec_end); ++a) {
      if (vec.size() == vec_end) {
      p_check:
        if (order(*a, *p) > 0) {
          vec.push_back(std::move(*a));
          *a = std::move(*p++);
        }
        continue;
      }
      if (p == c) {
      v_check:
        if (order(*a, vec[vec_end]) > 0) {
          vec.push_back(std::move(*a));
          *a = std::move(vec[vec_end++]);
        }
        continue;
      }
      if (order(vec[vec_end], *p) < 0) goto v_check;
      else goto p_check;
    }
    if (a != b) return;
    std::move(vec.begin() + vec_end, vec.end(), b);
    a = b;
    b = p;
  }
}
} // namespace ivl::algorithm
