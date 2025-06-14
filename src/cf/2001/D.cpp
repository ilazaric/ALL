#include <cassert>
#include <deque>
#include <ivl/io/conversion>
#include <ivl/io/stlutils.hpp>
#include <ivl/logger>
#include <ivl/structs/turnament.hpp>
#include <map>
#include <ranges>
#include <set>

struct Node {
  uint32_t maks;
  uint32_t mini;
  Node() : maks(0), mini(1e6) {}
  Node(uint32_t val) : maks(val), mini(val) {}
  Node(uint32_t maks, uint32_t mini) : maks(maks), mini(mini) {}
  bool operator==(const Node&) const = default;
};
Node operator+(Node a, Node b) {
  return {std::max(a.maks, b.maks), std::min(a.mini, b.mini)};
}

void one() {
  std::vector<uint32_t> a {cin};
  uint32_t              n = a.size();
  std::vector<uint32_t> suffix_uniq_cnts(n + 1);
  {
    suffix_uniq_cnts[n] = 0;
    std::vector<bool> seen(n + 1, false);
    for (auto idx : std::views::iota(0u, n) | std::views::reverse) {
      suffix_uniq_cnts[idx] = suffix_uniq_cnts[idx + 1] + (seen[a[idx]] ? 0 : 1);
      seen[a[idx]]          = true;
    }
  }
  suffix_uniq_cnts.pop_back();

  ivl::structs::Turnament<Node> tur(a);

  uint32_t target_len = suffix_uniq_cnts[0];

  std::set<std::pair<uint32_t, uint32_t>> last_locs;
  std::vector<std::vector<uint32_t>>      locs(n + 1);
  for (auto idx : std::views::iota(0u, n))
    locs[a[idx]].push_back(idx);
  for (auto idx : std::views::iota(0u, n + 1))
    if (!locs[idx].empty())
      last_locs.insert({locs[idx].back(), idx});

  uint32_t              lo = 0;
  std::vector<uint32_t> ans;
  for (auto bla : std::views::iota(0u, target_len)) {
    while (true) {
      auto cand = bla % 2 ? tur.find(lo, n, [&](Node node) { return node.mini < a[lo]; })
                          : tur.find(lo, n, [&](Node node) { return node.maks > a[lo]; });
      if (!cand)
        break;
      if (*cand > last_locs.begin()->first)
        break;
      lo = *cand;
    }
    ans.push_back(a[lo]);
    last_locs.erase({locs[a[lo]].back(), a[lo]});
    for (auto x : locs[a[lo]])
      tur.set(x, Node());
    while (lo < n && tur.leaf(lo) == Node())
      ++lo;
  }

  std::cout << target_len << std::endl;
  std::cout << ivl::io::Elems {ans} << std::endl;
}

int main() {
  for (uint32_t t {cin}; t--;) {
    one();
  }

  return 0;
}
