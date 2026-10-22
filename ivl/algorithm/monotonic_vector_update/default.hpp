#pragma once

#include "monotonic_join"
#include <algorithm>
#include <compare>
#include <vector>

template<typename Comp, typename T, typename Order = std::strong_ordering>
concept compare3_for = requires(const T& t, Comp c) {
  { c(t, t) } -> std::same_as<Order>;
};

namespace ivl::algorithm {
// Vectors are assumed to be sorted wrt. three-way comparison `order` defines.
// Each element represents a key-value pair of sorts.
// Outcome of function is a joined/concatenated-and-sorted vector, outputed in `a`.
// In case of same keys, `transform_or_skip()` is invoked, to either combine them inplace,
// or return `true` if this key should be dropped altogether.
template<typename T>
void monotonic_vector_update(std::vector<T>& a, const std::vector<T>& b, auto&& order, auto&& transform_or_skip) {
  if (a.empty()) {
    a = b;
    return;
  }
  if (b.empty()) return;

  auto ap = a.begin();
  auto bp = b.begin();
  auto last_slot = a.begin();
  auto end = a.end();
  while (ap != end && bp != b.end()) {
    auto cmp = order(*ap, *bp);
    if (cmp < 0) {
      if (ap != last_slot) *last_slot = std::move(*ap);
      ++ap;
      ++last_slot;
      continue;
    }
    if (cmp > 0) {
      a.push_back(*bp);
      ++bp;
      continue;
    }
    if (transform_or_skip(*ap, *bp)) {
      ++ap;
      ++bp;
    } else {
      if (ap != last_slot) *last_slot = std::move(*ap);
      ++ap;
      ++bp;
      ++last_slot;
    }
  }

  if (ap != last_slot) {
    std::move_backward(ap, a.end(), last_slot);
    a.erase(a.end() - (ap - last_slot), a.end());
    end -= ap - last_slot;
  }
  a.insert(a.end(), bp, b.end());
  monotonic_join(a.begin(), end, a.end(), order);
}
} // namespace ivl::algorithm
