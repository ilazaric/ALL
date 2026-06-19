#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

std::size_t choose(std::size_t a, std::size_t b) {
  static std::vector<std::vector<std::size_t>> cache;
  if (a < b) return 0;
  if (b == 0 || b == a) return 1;
  while (cache.size() <= a) cache.push_back({1});
  auto& row = cache[a];
  while (row.size() <= b) row.emplace_back(choose(a - 1, row.size() - 1) + choose(a - 1, row.size()));
  return row[b];
}

std::size_t encode_stupid(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::size_t ret = 0;
  std::size_t last = 0;
  for (std::size_t i = 0; i < m; ++i) {
    for (std::size_t j = last; j < data[i]; ++j) ret += choose(m - i - 1 + n - j, m - i - 1);
    last = data[i];
  }
  return ret;
}

std::size_t encode_smart(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::size_t ret = 0;
  std::size_t last = 0;
  for (std::size_t i = 0; i < m; ++i) {
    ret += choose(m - i + n - last, m - i);
    ret -= choose(m - i + n - data[i], m - i);
    last = data[i];
  }
  return ret;
}

std::size_t encode_smart2(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::size_t ret = 0;
  ret += choose(m + n, m);
  for (std::size_t i = 1; i < m; ++i) {
    ret += choose(m - i + n - data[i - 1], m - i);
  }
  for (std::size_t i = 0; i < m; ++i) {
    ret -= choose(m - i + n - data[i], m - i);
  }
  return ret;
}

std::size_t encode_smart3(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::size_t ret = choose(m + n, m) - choose(m + n - data[0], m);
  for (std::size_t i = 0; i + 1 < m; ++i) {
    ret += choose(m - i - 1 + n - data[i], m - i - 1);
  }
  for (std::size_t i = 1; i < m; ++i) {
    ret -= choose(m - i + n - data[i], m - i);
  }
  return ret;
}

std::size_t encode_smart4(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::size_t ret = choose(m + n, m);
  for (std::size_t i = 0; i < m; ++i) {
    ret += choose(m - i - 1 + n - data[i], m - i - 1);
  }
  ret -= choose(m - (m - 1) - 1 + n - data[m - 1], m - (m - 1) - 1);
  for (std::size_t i = 0; i < m; ++i) {
    ret -= choose(m - i + n - data[i], m - i);
  }
  return ret;
}

std::size_t encode_smart5(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::size_t ret = choose(m + n, m);
  ret -= choose(n - data[m - 1], 0);
  for (std::size_t i = 0; i < m; ++i) {
    ret += choose(m - i - 1 + n - data[i], m - i - 1);
    ret -= choose(m - i + n - data[i], m - i);
  }
  return ret;
}
