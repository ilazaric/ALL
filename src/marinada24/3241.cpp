#include <ivl/io/stlutils.hpp>
#include <ivl/logger>
#include <ranges>
#include <vector>

using State = std::array<double, 4>;

double eval(const State& a) {
  double x = (a[0] - a[1]) * (a[1] - a[2]) * (a[2] - a[3]) * (a[3] - a[0]);
  double y = a[0] * a[0] + a[1] * a[1] + a[2] * a[2] + a[3] * a[3];
  return x / y / y;
}

State chase() {
  State  a {1, -1, 0, 1};
  double anneal = 0.95;
  double last   = eval(a);
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
    anneal *= 0.95;
  }
  return a;
}

int main() {
  auto a = chase();
  LOG(eval(a));
}
