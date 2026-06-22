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

std::size_t encode_smart6(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::size_t ret = choose(m + n, m) - 1;
  for (std::size_t i = 0; i < m; ++i) {
    ret -= choose(m - i + n - data[i] - 1, m - i);
  }
  return ret;
}

std::size_t comb_count_smart(std::size_t n, std::size_t m) { return m == (std::size_t)-1 ? 0 : choose(n + m, n); }

std::size_t encode_weird(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::vector<std::size_t> counts(n + 1, 0);
  for (auto el : data) ++counts[el];
  for (std::size_t i = 0; i < n; ++i) counts[i + 1] += counts[i];
  std::size_t ret = 0;
  for (std::size_t i = 0; i <= n; ++i) {
    if (counts[i] == m) break;
    ret += comb_count_smart(n - i, m - counts[i] - 1);
  }
  return ret;
}

std::size_t encode_weird2(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::vector<std::size_t> counts(n + 1, 0);
  for (auto el : data) ++counts[el];
  for (std::size_t i = 0; i < n; ++i) counts[i + 1] += counts[i];
  std::size_t ret = 0;
  for (std::size_t i = 0; i < n; ++i) {
    ret += choose(n - i + m - 1 - counts[i], n - i);
  }
  return ret;
}

std::size_t encode_weird3(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::vector<std::size_t> counts(n + 1, 0);
  for (auto el : data) ++counts[el];
  for (std::size_t i = n; i; --i) counts[i - 1] += counts[i];
  std::size_t ret = 0;
  for (std::size_t i = 0; i < n; ++i) {
    ret += choose(n - i - 1 + counts[i + 1], n - i);
  }
  return ret;
}

std::size_t encode_weird4(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::vector<std::size_t> counts(n + 1, 0);
  for (auto el : data) ++counts[n - el];
  for (std::size_t i = 0; i < n; ++i) counts[i + 1] += counts[i];
  std::size_t ret = 0;
  for (std::size_t i = 0; i < n; ++i) {
    ret += choose(n - i - 1 + counts[n - i - 1], n - i);
  }
  return ret;
}

std::size_t encode_weird5(std::size_t n, std::size_t m, std::vector<std::size_t> data) {
  contract_assert(data.size() == m);
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::vector<std::size_t> counts(n + 1, 0);
  for (auto el : data) ++counts[n - el];
  for (std::size_t i = 0; i < n; ++i) counts[i + 1] += counts[i];
  std::size_t ret = 0;
  for (std::size_t i = 0; i < n; ++i) {
    ret += choose(i + counts[i], i + 1);
  }
  return ret;
}

std::vector<std::size_t> decode_first(std::size_t n, std::size_t m, std::size_t enc) {
  contract_assert(enc < comb_count_smart(n, m));
  std::size_t last = 0;
  std::vector<std::size_t> ret(m);
  for (std::size_t i = 0; i < m; ++i) {
    while (last < n) {
      auto foo = comb_count_smart(n - last, m - i - 1);
      if (enc < foo) break;
      enc -= foo;
      ++last;
    }
    ret[i] = last;
  }
  return ret;
}

std::size_t generic_encode_weird5(std::size_t n, std::vector<std::size_t> data) {
  std::ranges::sort(data);
  contract_assert(data.empty() || data.back() <= n);
  std::vector<std::size_t> counts(n + 1, 0);
  for (auto el : data) ++counts[n - el];
  for (std::size_t i = 0; i < n; ++i) counts[i + 1] += counts[i];
  std::size_t ret = 0;
  for (std::size_t i = 0; i < n; ++i) {
    ret += choose(i + counts[i], i + 1);
  }
  return ret;
}

std::vector<std::size_t> generic_decode_weird5(std::size_t n, std::size_t enc) {
  contract_assert(n != 0);
  std::vector<std::size_t> counts(n + 1, 0);
  std::vector<std::size_t> ret;
  for (std::size_t i = n - 1; i + 1; --i) {
    while (enc >= choose(i + 1 + counts[i], i + 1)) ++counts[i];
    enc -= choose(i + counts[i], i + 1);
    // for (std::size_t j = counts[i + 1]; j < counts[i]; ++j) ret.push_back(n - i);
    // LOG(i, counts[i]);
  }
  for (std::size_t i = n - 1; i + 1; --i) {
    std::size_t cnt = counts[i] - (i ? counts[i - 1] : 0);
    for (std::size_t j = 0; j < cnt; ++j) ret.push_back(n - i);
  }
  contract_assert(enc == 0);
  return ret;
}

std::size_t width(std::size_t n, std::size_t x) {
  std::size_t m = 0;
  while (choose(n + m, n) <= x) ++m;
  return m;
}

std::size_t count(std::size_t n, std::size_t enc, std::size_t digit) {
  contract_assert(digit <= n);
  contract_assert(digit != 0);
  std::size_t bigw = 0;
  while (digit) {
    bigw = width(n, enc);
    enc -= comb_count_smart(n, bigw - 1);
    --n;
    --digit;
  }
  return bigw - width(n, enc);
}

struct state {
  std::size_t n;

  std::size_t width(std::size_t x) const {
    std::size_t m = 0;
    while (choose(n + m, n) <= x) ++m;
    return m;
  }

  std::size_t encode(std::span<const std::size_t> dec) const {
    std::vector<std::size_t> counts(n + 1, 0);
    for (auto el : dec) {
      contract_assert(el <= n);
      ++counts[n - el];
    }
    for (std::size_t i = 0; i < n; ++i) counts[i + 1] += counts[i];
    std::size_t ret = 0;
    for (std::size_t i = 0; i < n; ++i) {
      ret += choose(i + counts[i], i + 1);
    }
    return ret;
  }

  std::vector<std::size_t> decode(std::size_t enc) const {
    contract_assert(n != 0);
    std::vector<std::size_t> counts(n + 1, 0);
    std::vector<std::size_t> ret;
    for (std::size_t i = n - 1; i + 1; --i) {
      while (enc >= choose(i + 1 + counts[i], i + 1)) ++counts[i];
      enc -= choose(i + counts[i], i + 1);
      // for (std::size_t j = counts[i + 1]; j < counts[i]; ++j) ret.push_back(n - i);
      // LOG(i, counts[i]);
    }
    for (std::size_t i = n - 1; i + 1; --i) {
      std::size_t cnt = counts[i] - (i ? counts[i - 1] : 0);
      for (std::size_t j = 0; j < cnt; ++j) ret.push_back(n - i);
    }
    contract_assert(enc == 0);
    return ret;
  }
};
