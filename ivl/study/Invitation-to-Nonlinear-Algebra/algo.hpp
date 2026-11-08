#pragma once

#include <algorithm>
#include <vector>

template<typename T>
void monotonic_transform(std::vector<T>& a, const std::vector<T>& b, auto order, auto transform_or_skip) {
  if (a.empty()) {
    a = b;
    return;
  }

  if (b.empty()) return;

  auto bp = b.begin();
  size_t ap = 0;
  size_t zoff = 0;
  size_t aq = 0;

no_zeroes_start:
  if (ap == a.size()) {
    a.insert(a.end(), bp, b.end());
    return;
  }
  for (aq = a.size(); ap < aq; ++ap) {
    auto cmp = order(a[ap], *bp);
    if (cmp < 0) continue;
    if (cmp > 0) {
      // TODO
      a.push_back(*bp);
      std::swap(a[ap], a.back());
    } else if (transform_or_skip(a[ap], *bp)) goto inc_zoff;
    rev_zoff:
    ++bp;
    if (bp == b.end()) {
      std::rotate(a.begin() + (ap + 1), a.begin() + aq, a.end());
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
    auto cmp = order(a[ap], *bp);
    if (cmp < 0) {
      a[ap-zoff] = std::move(a[ap]);
      continue;
    }
    if (cmp > 0) {
      a[ap-zoff] = *bp;
      --zoff;
      if (zoff == 0) goto rev_zoff;
    } else if (transform_or_skip(a[ap], *bp)) inc_zoff: ++zoff;
    else a[ap-zoff] = std::move(a[ap]);
    ++bp;
    if (bp == b.end()) {
      ++ap;
      // [zoff] [aq-ap] [asz-aq]
      if (zoff < a.size() - aq) {
        std::move(a.begin() + aq, a.begin() + (aq + zoff), a.begin() + (ap - zoff));
        std::move_backward(a.begin() + (ap + 1), a.begin() + (ap + 1), a.begin() + aq);
        std::move();
      } else {
      }
      std::rotate(a.begin() + (ap + 1), a.begin() + aq, a.end());
      return;
    }
  }
  goto yes_zeroes_start;
}
