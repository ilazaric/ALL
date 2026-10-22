#pragma once

#include <ivl/logger>
#include <algorithm>
#include <compare>
#include <span>
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

// O(N) time
// O(N) memory
template<typename It>
void monotonic_join_v2(It a, It b, It c) {
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

template<typename It>
void monotonic_join(It a, It b, It c) {
  return monotonic_join_2(a, b, c);
}
} // namespace ivl::algorithm
