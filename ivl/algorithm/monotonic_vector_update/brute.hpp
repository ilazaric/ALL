#pragma once

#include "optional"
#include <algorithm>
#include <optional>
#include <vector>

namespace ivl::algorithm {
template<typename T>
void monotonic_vector_update_brute(std::vector<T>& a, const std::vector<T>& b, auto&& order, auto&& hook) {
  auto ap = a.begin();
  auto bp = b.begin();
  std::vector<T> ret;
  auto left = [&] {
    monotonic_vector_update_optional opt{a, (size_t)(ap - a.begin()), true};
    hook(opt, std::optional<const T&>(std::nullopt));
    if (opt) ret.emplace_back(std::move(*ap));
    ++ap;
  };
  auto right = [&] {
    monotonic_vector_update_optional opt{ret, ret.size(), false};
    hook(opt, std::optional<const T&>(*bp));
    if (!opt && opt.idx != ret.size()) ret.pop_back();
    ++bp;
  };
  while (ap != a.end() && bp != b.end()) {
    auto cmp = order(*ap, *bp);
    if (cmp < 0) {
      left();
      continue;
    }
    if (cmp > 0) {
      right();
      continue;
    }
    monotonic_vector_update_optional opt{a, (size_t)(ap - a.begin()), true};
    hook(opt, std::optional<const T&>(*bp));
    if (opt) ret.emplace_back(std::move(*ap));
    ++ap;
    ++bp;
  }
  while (ap != a.end()) left();
  while (bp != b.end()) right();
  a = std::move(ret);
}
} // namespace ivl::algorithm
