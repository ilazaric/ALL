#pragma once

#include <algorithm>
#include <compare>
#include <vector>

namespace ivl::algorithm {
// Vectors are assumed to be sorted wrt. three-way comparison `order` defines.
// Each element represents a key-value pair of sorts.
// Outcome of function is a joined/concatenated-and-sorted vector, outputed in `a`.
// In case of same keys, `transform_or_skip()` is invoked, to either combine them inplace,
// or return `true` if this key should be dropped altogether.
template<typename T>
void monotonic_vector_join(std::vector<T>& a, const std::vector<T>& b, auto&& order, auto&& transform_or_skip) {
  if (a.empty()) {
    a = b;
    return;
  }

  if (b.empty()) return;

  auto bp = b.begin();
  size_t ap = 0;
  size_t zoff = 0;
  size_t aq = 0;
  // size_t ar = 0;
  std::strong_ordering cmp = std::strong_ordering::equivalent;

no_zeroes_start:
  if (ap == a.size()) {
    a.insert(a.end(), bp, b.end());
    return;
  }
  for (aq = a.size(); ap < aq; ++ap) {
    cmp = order(a[ap], *bp);
    if (cmp < 0) continue;
    if (cmp > 0) {
      a.push_back(std::move(a[ap]));
      a[ap] = *bp;
    } else if (transform_or_skip(a[ap], *bp)) goto inc_zoff;
  rev_zoff:
    ++bp;
    if (bp == b.end()) {
      ++ap;
      std::rotate(a.begin() + ap, a.begin() + aq, a.end());
      return;
    }
  }
  goto no_zeroes_start;

yes_zeroes_start:
  if (ap == a.size()) {
    a.erase(a.end() - zoff, a.end());
    a.insert(a.end(), bp, b.end());
    return;
  }
  for (aq = a.size(); ap < aq; ++ap) {
    cmp = order(a[ap], *bp);
    if (cmp < 0) {
      a[ap - zoff] = std::move(a[ap]);
      continue;
    }
    if (cmp > 0) {
      a[ap - zoff] = *bp;
      --zoff;
      if (zoff == 0) goto rev_zoff;
    } else if (transform_or_skip(a[ap], *bp))
    inc_zoff:
      ++zoff;
    else a[ap - zoff] = std::move(a[ap]);
    ++bp;
    if (bp == b.end()) {
      ++ap;
      // [zoff] ap [aq-ap] aq [asz-aq] end
      if (zoff < a.size() - aq) {
        std::move(a.begin() + aq, a.begin() + (aq + zoff), a.begin() + (ap - zoff));
        std::move_backward(a.begin() + ap, a.begin() + aq, a.begin() + (ap - zoff + a.size() - aq));
        std::move(a.begin() + (aq + zoff), a.end(), a.begin() + aq);
      } else {
        std::move(a.begin() + aq, a.end(), a.begin() + (ap - zoff));
        std::move(a.begin() + ap, a.begin() + aq, a.begin() + (ap - zoff + a.size() - aq));
      }
      a.resize(a.size() - zoff);
      return;
    }
  }
  goto yes_zeroes_start;
}
} // namespace ivl::algorithm
