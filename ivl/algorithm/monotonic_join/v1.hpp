#pragma once

#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>

namespace ivl::algorithm {
// O(N) time
// O(N) memory
template<typename It>
void monotonic_join_v1(It a, It b, It c) {
  while (a != b && b != c) {
    std::vector<std::remove_cvref_t<decltype(*a)>> vec;
    size_t vec_end = 0;
    It p = b;
    for (; a != b && (p != c || vec.size() != vec_end); ++a) {
      if (vec.size() == vec_end) {
      p_check:
        if (*a > *p) {
          vec.push_back(std::move(*a));
          *a = std::move(*p++);
        }
        continue;
      }
      if (p == c) {
      v_check:
        if (*a > vec[vec_end]) {
          vec.push_back(std::move(*a));
          *a = std::move(vec[vec_end++]);
        }
        continue;
      }
      if (vec[vec_end] < *p) goto v_check;
      else goto p_check;
    }
    if (a != b) return;
    std::move(vec.begin() + vec_end, vec.end(), b);
    a = b;
    b = p;
  }
}
} // namespace ivl::algorithm
