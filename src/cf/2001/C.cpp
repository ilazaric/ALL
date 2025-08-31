#include <ivl/io/conversion>
#include <ivl/io/stlutils.hpp>
#include <ivl/logger>
#include <map>
#include <ranges>

uint32_t query(uint32_t a, uint32_t b) {
  std::cout << "? " << a + 1 << " " << b + 1 << std::endl;
  return (int)cin - 1;
}

void answer(const auto& edges) {
  std::cout << "!";
  for (auto idx : std::views::iota(0u, edges.size()))
    for (auto el : edges[idx])
      std::cout << " " << idx + 1 << " " << el + 1;
  std::cout << std::endl;
}

void one() {
  uint32_t                           n{cin};
  std::vector<std::vector<uint32_t>> edges(n);

  std::vector<bool> seen(n, false);
  seen[0] = true;

  uint32_t curr = 0;
  while (curr < n) {
    LOG(curr, seen[curr]);
    if (seen[curr]) {
      ++curr;
      continue;
    }
    uint32_t start = 0;
    uint32_t end   = curr;
    while (true) {
      auto ans = query(start, end);
      if (ans == start) break;
      if (seen[ans]) start = ans;
      else end = ans;
    }
    edges[start].push_back(end);
    seen[end] = true;
  }

  answer(edges);
}

int main() {
  for (uint32_t t{cin}; t--;) {
    one();
  }

  return 0;
}
