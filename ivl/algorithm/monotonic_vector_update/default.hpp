#pragma once

#include <ivl/algorithm/monotonic_join>
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

  auto ap = 0;
  auto bp = b.begin();
  auto last_slot = 0;
  auto end = a.size();
  while (ap != end && bp != b.end()) {
    auto cmp = order(a[ap], *bp);
    if (cmp < 0) {
      if (ap != last_slot) a[last_slot] = std::move(a[ap]);
      ++ap;
      ++last_slot;
      continue;
    }
    if (cmp > 0) {
      a.push_back(*bp);
      ++bp;
      continue;
    }
    if (transform_or_skip(a[ap], *bp)) {
      ++ap;
      ++bp;
    } else {
      if (ap != last_slot) a[last_slot] = std::move(a[ap]);
      ++ap;
      ++bp;
      ++last_slot;
    }
  }

  if (ap != last_slot) {
    end -= ap - last_slot;
    std::move(a.begin() + ap, a.end(), a.begin() + last_slot);
    a.erase(a.end() - (ap - last_slot), a.end());
  }
  a.insert(a.end(), bp, b.end());
  monotonic_join(a.begin(), a.begin() + end, a.end(), order);
}
} // namespace ivl::algorithm
