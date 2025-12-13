#include <ivl/io/stlutils>
#include <ivl/logger>
#include <ranges>
#include <vector>

double eval(const std::vector<double>& a) {
  double bla  = 0;
  double truc = 0;
  for (auto i : std::views::iota(0u, a.size()))
    for (auto j : std::views::iota(0u, a.size()))
      bla += a[i] * a[j] * ((int)a.size() - abs(((int)i - (int)j)));
  for (auto i : std::views::iota(0u, a.size()))
    truc += a[i] * a[i];
  return bla / truc;
}

std::vector<double> chase(uint32_t n) {
  std::vector<double> a(n, 1);
  double              anneal = 0.8;
  double              last   = eval(a);
  for (auto _ : std::views::iota(0, 5000)) {
    for (auto& x : a) {
      auto e1 = last;
      x += anneal;
      auto e2 = eval(a);
      if (e2 < e1) {
        last = e2;
        continue;
      }
      x -= 2 * anneal;
      auto e3 = eval(a);
      if (e3 < e1) {
        last = e3;
        continue;
      }
      x += anneal;
    }
    anneal *= 0.98;
  }
  return a;
}

int main() {
  for (auto n : std::views::iota(60, 70)) {
    auto a = chase(n);
    LOG(eval(a), a);
  }
}
