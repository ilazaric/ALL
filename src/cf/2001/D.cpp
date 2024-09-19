#include <ivl/logger>
#include <ivl/io/conversion>
#include <ivl/io/stlutils.hpp>
#include <map>
#include <ranges>
#include <deque>
#include <set>
#include <cassert>

struct Node {
  int maks;
  int mini;
  bool operator==(const Node&) const = default;
};
const Node DEAD{-1, (int)1e6};
Node operator+(Node a, Node b){
  if (a == DEAD) return b;
  if (b == DEAD) return a;
  return {std::max(a.maks, b.maks),
          std::min(a.mini, b.mini)};
}

constexpr uint32_t MAXLEN = 1<<19;
Node tur[MAXLEN*2];

void refresh(uint32_t id){
  tur[id] = tur[id*2] + tur[id*2+1];
}

void set(uint32_t pos, Node node){
  tur[pos + MAXLEN] = node;
  for (uint32_t id = (pos + MAXLEN) / 2; id; id /= 2)
    refresh(id);
}

void set(uint32_t pos, int val){
  set(pos, Node{val, val});
}

void unset(uint32_t pos){
  set(pos, DEAD);
}

constexpr uint32_t NONE = (uint32_t)-1;

uint32_t first_less(uint32_t id,
                    uint32_t lo,
                    uint32_t hi,
                    uint32_t slo,
                    uint32_t shi,
                    int val){
  if (hi < slo) return NONE;
  if (lo > shi) return NONE;
  if (tur[id].mini >= val) return NONE;
  if (lo == hi) return tur[id].maks < val ? lo : NONE;
  auto mid = (lo+hi)/2;
  auto left = first_less(id*2, lo, mid, slo, shi, val);
  if (left != NONE) return left;
  return first_less(id*2+1, mid+1, hi, slo, shi, val);
}

uint32_t first_less(uint32_t lo, uint32_t hi, int val){
  return LOG(lo, hi, val, first_less(1, 0, MAXLEN-1, lo, hi, val));
}

uint32_t first_gt(uint32_t id,
                  uint32_t lo,
                  uint32_t hi,
                  uint32_t slo,
                  uint32_t shi,
                  int val){
  if (hi < slo) return NONE;
  if (lo > shi) return NONE;
  if (tur[id].maks <= val) return NONE;
  if (lo == hi) return tur[id].mini > val ? lo : NONE;
  // if (dbg) LOG(id, lo, hi, slo, shi, val, tur[id].maks, tur[id].mini);
  auto mid = (lo+hi)/2;
  auto left = first_gt(id*2, lo, mid, slo, shi, val);
  if (left != NONE) return left;
  return first_gt(id*2+1, mid+1, hi, slo, shi, val);
}

uint32_t first_gt(uint32_t lo, uint32_t hi, int val){
  LOG(lo, hi, val);
  return first_gt(1, 0, MAXLEN-1, lo, hi, val);
}

void one(){

  std::vector<uint32_t> a{cin};
  uint32_t n = a.size();
  std::vector<uint32_t> suffix_uniq_cnts(n+1); {
    suffix_uniq_cnts[n] = 0;
    std::vector<bool> seen(n+1, false);
    for (auto idx : std::views::iota(0u, n) | std::views::reverse){
      suffix_uniq_cnts[idx] = suffix_uniq_cnts[idx+1] + (seen[a[idx]] ? 0 : 1);
      seen[a[idx]] = true;
    }
  }
  suffix_uniq_cnts.pop_back();

  for (auto idx : std::views::iota(0u, n))
    set(idx, a[idx]);

  // LOG(first_gt(0, n-1, a[0]));
  // return;

  uint32_t target_len = suffix_uniq_cnts[0];

  std::set<std::pair<uint32_t, uint32_t>> last_locs;
  std::vector<std::vector<uint32_t>> locs(n+1);
  for (auto idx : std::views::iota(0u, n))
    locs[a[idx]].push_back(idx);
  for (auto idx : std::views::iota(0u, n+1))
    if (!locs[idx].empty())
      last_locs.insert({locs[idx].back(), idx});

  uint32_t lo = 0;
  std::vector<uint32_t> ans;
  for (auto bla : std::views::iota(0u, target_len)){
    for (int i = 0; i < n; ++i) LOG(i, tur[i+MAXLEN].maks, tur[i+MAXLEN].mini);
    while (true){
      auto cand = bla % 2 ? first_less(lo, n-1, a[lo]) : first_gt(lo, n-1, a[lo]);
      LOG(lo, cand);
      assert(cand != lo);
      if (cand == NONE) break;
      if (cand > last_locs.begin()->first) break;
      lo = cand;
    }
    LOG(bla, lo);
    ans.push_back(a[lo]);
    last_locs.erase({locs[a[lo]].back(), a[lo]});
    for (auto x : locs[a[lo]])
      unset(LOG(x));
    while (lo < n && tur[lo+MAXLEN].maks == -1) ++lo;
  }

  std::cout << target_len << std::endl;
  std::cout << ivl::io::Elems{ans} << std::endl;

}

int main(){

  for (uint32_t t{cin}; t--;){
    one();
  }

  return 0;
}
