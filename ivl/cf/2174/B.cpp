#include <iostream>
#include <vector>

void solve() {
  int n, k;
  std::cin >> n >> k;
  std::vector<int> a(n);
  for (auto& ai : a) std::cin >> ai;

  std::vector<int> inc_seq;
  {
    int last = 0;
    for (int i = 0; i < n; ++i)
      if (a[i] > last) {
        inc_seq.push_back(i);
        last = a[i];
      }
  }

  if (inc_seq.empty()) {
    std::cout << 0 << std::endl;
    return;
  }

  std::vector<int> dist(inc_seq.size());
  for (int i = 0; i + 1 < inc_seq.size(); ++i) dist[i] = inc_seq[i + 1] - inc_seq[i];
  dist.back() = n - inc_seq.back();

  std::vector cache(inc_seq.size() + 1, std::vector(k + 1, std::vector(k + 1, 0)));
  std::vector diag_cache(inc_seq.size() + 1, std::vector(k + 1, 0));

  // diag_cache[i, rem] = max(cache[i, X, rem-X])

  for (int i = inc_seq.size() - 1; i >= 0; --i)
    for (int rem = 0; rem <= k; ++rem)
      for (int lmax = 0; lmax <= k; ++lmax) {
        cache[i][lmax][rem] = std::max(dist[i] * lmax + cache[i + 1][lmax][rem], diag_cache[i][rem]);
        if (rem + lmax <= k && lmax <= a[inc_seq[i]])
          diag_cache[i][rem + lmax] = std::max(diag_cache[i][rem + lmax], cache[i][lmax][rem]);
      }

  std::cout << cache[0][0][k] << std::endl;
}

int main() {
  int t;
  std::cin >> t;
  for (int i = 0; i < t; ++i) solve();
}
