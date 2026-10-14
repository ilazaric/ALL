#pragma once

#include <ivl/logger>
#include <algorithm>
#include <compare>
#include <span>
#include <utility>
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

template<typename It>
void monotonic_contiguous_join_v4(It a, It b, It c) {
  if (a == b || b == c) return;
  if (*(b - 1) <= *b) return;
  while (*a <= *b) ++a;
  while (*(c - 1) >= *(b - 1)) --c;

  if (c - b > 3) return monotonic_contiguous_join_v1(a, b, c);

  std::vector<std::pair<size_t, size_t>> rots;

  It low = a;
  for (It high = b; high != c; ++high) {
    while (*low <= *high) ++low;
    auto x = low - a + (high - b);
    auto y = high - a;
    rots.emplace_back(x, y);
  }

  auto rot = [&](size_t lo, size_t hi, size_t n) {
    contract_assert(lo <= hi);
    if (lo == hi) return;
    std::rotate(a + lo, a + hi - n % (hi - lo), a + hi);
  };

  auto swp = [&](size_t lo, size_t hi) { std::iter_swap(a + lo, a + hi); };

  if (rots.size() == 1) {
    auto [x, y] = rots[0];
    rot(x, y + 1, 1);
  }

  if (rots.size() == 2) {
    auto [x, y] = rots[0];
    auto [z, w] = rots[1];
    rot(x, z, 1);
    swp(x, y);
    rot(z, y + 1, 2);
    swp(z, w);
  }

  if (rots.size() == 3) {
    auto [x, y] = rots[0];
    auto [z, w] = rots[1];
    auto [t, u] = rots[2];
    rot(x, z, 1);
    swp(x, y);
    if (y < t) {
      if (z == y) swp(y, w);
      else swp(y - 1, w);
      rot(z, y + 1, 2);
      rot(t, u + 1, 1);
    } else {
      if (z == y) swp(y, w);
      else swp(y - 1, w);
      rot(z, t, 2);
      if (y - z == 1) {
      } else if (t - z == 1) {
        swp(z, y - 1);
        swp(y - 1, y);
      } else if (y - t == 0) {
        swp(z, y);
        swp(z, z + 1);
      } else {
        swp(z, y - 1);
        swp(z + 1, y);
      }
      rot(t, y + 1, 3);
      swp(t, u);
      rot(y + 1, u + 1, 1);
    }
  }

  // (a b c ... x y z)
  // (a b c ... x y) (a z)
  // (y z) (a b c ... x y)

  // (a b c ... x y z)^N
  // [ (a b c ... x y) (a z) ]^N
  // (a b c ... x y)^N f^(N-1) (a z) f^(N-2) (a z) ... f^(0) (a z)
  // f(p) = c^-1 p c
  // f^X(a z) = (c^X(a) z)

  // (a b c ... l m n o ... x y z)
  // (a b c ... l m) (n o ... x y z a)
  // (a b c ... l m z) (n o ... x y z)

  // (0 1 2 ... N-3 N-2 N-1)
  // (0 1 2 ... N-3 N-2 N-1)^A
  // (0 A 2A ... A(N-3) A(N-2) A(N-1))

  // (a z) (b z) (c z) (d z)
  // (a b c d z)

  // (a b c ... l m n ... x y z) (m n ... x y z w)
  // (z a b c ... l) (m n ... x y z) (m n ... x y z) (w m)
  // (z a b c ... l) (m n ... x y z)^2 (w m)
  // (z a b c ... l) (w y) (m n ... x y z)^2

  // (a b c ... l m n ... x y z)^N (m n ... x y z w)
  // (a b c ... l m n ... x y z)^N (w z) (m n ... x y z)
  // (w z)conjN (a b c ... l m n ... x y z)^N (m n ... x y z)
  // (w z)conjN (a b c ... l m n ... x y z)^N (a b c ... l z)^-1 (a b c ... l z) (m n ... x y z)

  // (a b c ... l m n ... x y z)^N
  // [ (a z) (b c ... l m n ... x y z) ]^N
  // (a z) (a z)conj (a z)conj2 ... (a z)conjN-1 (b c ... l m n ... x y z) ]^N
  // (a z) (a y) (a x) ... (a z_prevN-1) (b c ... l m n ... x y z) ]^N
  // (a z y x ... z_prevN-1) (b c ... l m n ... x y z) ]^N
  // (a z y x ... z_prevN-1) (b z y x ... z_prevN-1) (c ... l m n ... x y z) ]^N
  // (a z_prevN-1) (z y x ... z_prevN-1) (b z_prevN-1)  (z y x ... z_prevN-1) (c ... l m n ... x y z) ]^N

  // (l m n ... p q r)
  // (a b c ... x y z) (a b c ... l)^-1 (r ... x y z)^-1 (z r) ?

  // (a b c ... l m n ... p q r ... x y z)
  // (a b c ... l q) (m n ... p q) (m r ... x y z)
  // (a b c ... l) (a q) (m n ... p q) (m z) (r ... x y z)

  // (m n ... p q)
  // [ (a b c ... l) (a q) ]^-1 (a b c ... x y z) [ (m z) (r ... x y z) ]^-1
  // (a q) (l ... a) (a b c ... x y z) (z ... r) (m z)

  // (0 ... m+k-1)^k . (a ... m+k) ;; a >= k
  // (0 ... m+k-1)^k . (a ... m+k) . (k ... m+k)^-1 . (k ... m+k)
  // (0 ... m+k-1)^k . (a ... m+k) . (k k+1 ... a-1 a a+1 ... m+k)^-1 . (k ... m+k)
  // (0 ... m+k-1)^k . (m+k a-1 a-2 ... k+1 k) . (k ... m+k)
  // (m a-k-1 a-k-2 ... 1 0) . (0 ... m+k-1)^k . (k ... m+k)
  // (m a-k-1 a-k-2 ... 1 0) . (0 ... m+k)^k+1
  // (m 0) . (a-k-1 ... 0) . (0 ... m+k)^k+1 ;; if a >  k
  //                         (0 ... m+k)^k+1 ;; if a == k

  // (a ... 0)^k (b ... 0) ;; b >= a
  // (a ... 0)^k+1         ;; b == a
  // (a ... 0)^k (b ... 0) ;; b >  a ?
  // (a ... 0)^k (b ... a+1 a ... 0)
  // (a ... 0)^k (b ... a+1 0) (a ... 0)
  // (b ... a+1 k%(a+1)) (a ... 0)^k (a ... 0) ;; a >= k so mod useless
  // (b ... a+1 k) (a ... 0)^k+1 ;; left is reasonably small, but right ought to shift to (b ... 0)^k+1 somehow

  // (0 ... a+b) (0 ... a)^-1
  // (a a+1 ... a+b)

  // (a ... a+b+k-1)^k . (a+k+offset ... a+b+k)
  // (a ... a+b+k)^k+1 ;; offset == 0
  // (a ... a+b+k-1)^k . (a+k+offset ... a+b+k) ;; offset > 0
  // (a ... a+b+k-1)^k . (a+k+offset ... a+b+k)
}
} // namespace ivl::algorithm
