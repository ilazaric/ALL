#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

#include <ivl/logger/logger>
using namespace ivl::logger::default_logger;

int main() {
  int              n = 1'000'000'000;
  std::vector<int> vec(n + 1);
  for (auto i : std::views::iota(2, n + 1)) {
    vec[i] = vec[i - 1];
    if (i % 2 == 0) vec[i] = std::min(vec[i], vec[i / 2]);
    if (i % 3 == 0) vec[i] = std::min(vec[i], vec[i / 3]);
    ++vec[i];
  }

  auto describe = [&](int i) {
    LOG(i);
    while (i != 1) {
      if (vec[i] == vec[i - 1] + 1) {
        LOG(i = i - 1);
      } else if (i % 2 == 0 && vec[i] == vec[i / 2] + 1) {
        LOG(i = i / 2);
      } else {
        LOG(i = i / 3);
      }
    }
  };

  auto opt = [&](int i) {
    return (vec[i] == vec[i - 1] + 1) + (i % 2 == 0 && vec[i] == vec[i / 2] + 1) * 2 +
           (i % 3 == 0 && vec[i] == vec[i / 3] + 1) * 4;
  };

  // describe(321);

  // LOG(321/3);
  // describe(321/3);

  if (0) {
    int best  = 1;
    int delta = -1;
    for (auto i : std::views::iota(1, n + 1))
      if (i % 3 == 0)
        if (vec[i / 3] - vec[i] > delta) best = i, delta = vec[i / 3] - vec[i];

    LOG(best, delta);
  }

  // if n % 6 == 0 we will not -1, have to /2 or /3
  if (0) {
    for (auto i : std::views::iota(1, n + 1))
      if (i % 2 == 0 && i % 3 == 0) {
        int wtv = n + 5;
        if (i % 2 == 0) wtv = std::min(wtv, vec[i / 2] + 1);
        if (i % 3 == 0) wtv = std::min(wtv, vec[i / 3] + 1);
        if (wtv != vec[i]) LOG(i, vec[i], wtv);
      }
  }

  if (0) {
    const int            Mod = 8 * 27;
    std::array<int, Mod> opts;
    std::ranges::fill(opts, 7);
    for (auto i : std::views::iota(2, n + 1))
      opts[i % Mod] &= opt(i);
    for (auto i : std::views::iota(0, Mod))
      if (i % 2 == 0 || i % 3 == 0)
        if (opts[i]) LOG(i, opts[i]);
  }

  for (auto i : std::views::iota(2, n + 1))
    if (i % 6 == 2)
      if (vec[i] == vec[i / 3] + 3) LOG(i);
}
