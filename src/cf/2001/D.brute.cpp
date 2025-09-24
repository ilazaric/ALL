#include <ivl/io/conversion>
#include <ivl/io/stlutils>
#include <ivl/logger>
#include <cassert>
#include <deque>
#include <map>
#include <ranges>
#include <set>

int main() {
  std::vector<uint32_t> a{cin};
  uint32_t              n = a.size();

  uint32_t target_len = std::set(a.begin(), a.end()).size();
  LOG(target_len);
  std::vector<uint32_t> ans;
  std::vector<char>     seen(n + 1, 0);
  uint32_t              start = 0;

  while (true) {
    while (start < n && seen[a[start]])
      ++start;
    if (start == n) break;

    for (int i = start; i < n; ++i) {
      if (seen[a[i]]) continue;
      for (auto j = i; j < n; ++j)
        if (!seen[a[j]]) seen[a[j]] = 2;
      uint32_t cnt = 0;
      for (int j = 0; j <= n; ++j) {
        if (seen[j]) ++cnt;
        if (seen[j] == 2) seen[j] = 0;
      }
      if (cnt != target_len) continue;
      if (ans.size() % 2 == 0 && a[start] < a[i]) start = i;
      if (ans.size() % 2 == 1 && a[start] > a[i]) start = i;
    }

    seen[a[start]] = 1;
    ans.push_back(a[start]);
  }

  std::cout << target_len << std::endl;
  std::cout << ivl::io::Elems{ans} << std::endl;
}
