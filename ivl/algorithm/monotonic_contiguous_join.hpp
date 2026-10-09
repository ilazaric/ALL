#pragma once

#include <algorithm>
#include <compare>
#include <span>
#include <vector>

namespace ivl::algorithm {
template<typename It>
void monotonic_contiguous_join_v1(It a, It b, It c) {
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

template<typename It>
void monotonic_contiguous_join_v2(It a, It b, It c) {
  if (a == b || b == c) return;
  std::vector<size_t> perm(c - a);
  {
    size_t i = 0;
    It p = a;
    It q = b;
    while (p != b && q != c) {
      if (*p <= *q) perm[i++] = p++ - a;
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
