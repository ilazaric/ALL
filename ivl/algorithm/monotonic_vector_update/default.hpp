#pragma once

#include <ivl/algorithm/monotonic_join>
#include "optional"
#include <algorithm>
#include <compare>
#include <optional>
#include <ranges>
#include <vector>

namespace ivl::algorithm {
template<typename T>
void monotonic_vector_update(std::vector<T>& a, const std::vector<T>& b, auto&& order, auto&& hook) {
  auto ap = 0UZ;
  auto bp = b.begin();
  auto last_slot = 0UZ;
  auto end = a.size();

  auto left = [&] {
    monotonic_vector_update_optional opt{a, ap, true};
    hook(opt, std::optional<const T&>(std::nullopt));
    if (opt) {
      if (ap != last_slot) a[last_slot] = std::move(a[ap]);
      ++last_slot;
    }
    ++ap;
  };

  auto right = [&] {
    monotonic_vector_update_optional opt{a, a.size(), false};
    hook(opt, std::optional<const T&>(*bp));
    if (!opt && a.size() != opt.idx) a.pop_back();
    ++bp;
  };

  while (ap != end && bp != b.end()) {
    auto cmp = order(a[ap], *bp);
    if (cmp < 0) {
      left();
      continue;
    }
    if (cmp > 0) {
      right();
      continue;
    }
    monotonic_vector_update_optional opt{a, ap, true};
    hook(opt, std::optional<const T&>(*bp));
    if (opt) {
      if (ap != last_slot) a[last_slot] = std::move(a[ap]);
      ++last_slot;
    }
    ++ap;
    ++bp;
  }

  while (ap != end) left();

  if (ap != last_slot) {
    end -= ap - last_slot;
    std::move(a.begin() + ap, a.end(), a.begin() + last_slot);
    a.erase(a.end() - (ap - last_slot), a.end());
  }
  while (bp != b.end()) right();
  monotonic_join(a.begin(), a.begin() + end, a.end(), order);
}
} // namespace ivl::algorithm
