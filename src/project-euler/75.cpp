#include <algorithm>

#include <ivl/logger>

// constexpr size_t maxl = 48;
constexpr size_t maxl = 1'500'000;

char clrs[maxl + 1];

// m2-n2, 2mn, m2+n2

size_t gcd(size_t a, size_t b) {
  while (b) {
    a %= b;
    std::swap(a, b);
  }
  return a;
}

int main() {
  for (size_t m = 1; m * m <= maxl; ++m)
    for (size_t n = 1; n < m; ++n) {
      if (gcd(m, n) != 1)
        continue;
      if (m % 2 == n % 2)
        continue;
      for (size_t d = m * m - n * n + 2 * m * n + m * m + n * n, s = d; s <= maxl; s += d) {
        // if (s == 24) LOG(m, n);
        if (clrs[s] != 2)
          ++clrs[s];
      }
    }

  LOG(std::ranges::count(clrs, 1));
  // LOG((int)clrs[24]);
  // for (size_t i = 0; i <= maxl; ++i)
  //   if (clrs[i] == 1)
  //     LOG(i);
}
