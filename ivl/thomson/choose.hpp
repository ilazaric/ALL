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
