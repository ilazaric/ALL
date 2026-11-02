#pragma once

#include <cstddef>
#include <mdspan>
#include <utility>
#include <vector>

namespace ivl::algorithm {
std::vector<std::pair<size_t, size_t>> longest_common_subsequence(size_t n, size_t m, auto&& pred) {
  std::vector<size_t> cache((n + 1) * (m + 1), 0);
  std::mdspan dp(cache.data(), n + 1, m + 1);
  for (size_t i = n - 1; i + 1; --i)
    for (size_t j = m - 1; j + 1; --j) {
      if (pred(i, j)) dp[i, j] = 1 + dp[i + 1, j + 1];
      else dp[i, j] = std::max(dp[i + 1, j], dp[i, j + 1]);
    }
  std::vector<std::pair<size_t, size_t>> ret;
  size_t i = 0, j = 0;
  while (i < n && j < m) {
    if (pred(i, j)) {
      ret.emplace_back(i++, j++);
      continue;
    }
    if (dp[i + 1, j] >= dp[i, j + 1]) ++i;
    else ++j;
  }
  return ret;
}
} // namespace ivl::algorithm
