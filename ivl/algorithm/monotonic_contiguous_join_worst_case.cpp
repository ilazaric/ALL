#include <ivl/format>
#include <ivl/utility/tsc>
#include "monotonic_contiguous_join"
#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>

template<auto impl>
struct state {
  std::mt19937 gen{1337};
  size_t n;
  std::vector<bool> curr;
  std::vector<bool> best;
  std::vector<size_t> data;
  ivl::tsc_duration best_duration;

  void mutate_flip(double p) {
    std::bernoulli_distribution dist(p);
    for (auto&& el : curr)
      if (dist(gen)) el = !el;
  }

  void mutate_shift(double p) {
    std::bernoulli_distribution dist(p);
    for (size_t i = 0; i + i < curr.size(); ++i)
      if (dist(gen)) swap(curr[i], curr[i + 1]);
  }

  ivl::tsc_duration run() {
    size_t foo = 0;
    size_t bar = 0;
    for (size_t i = 0; i < n; ++i) {
      if (curr[i]) data[foo++] = i;
      else data[n - 1 - (bar++)] = i;
    }
    std::reverse(data.begin() + foo, data.end());
    auto start = ivl::tsc_now();
    [:impl:](data.begin(), data.begin() + foo, data.end());
    asm volatile("" ::"m"(data));
    auto end = ivl::tsc_now();
    return end - start;
  }

  bool try_run() {
    auto dur = run();
    if (dur > best_duration) {
      best = curr;
      best_duration = dur;
      return true;
    } else {
      curr = best;
      return false;
    }
  }

  void find_worst_case(size_t n_, size_t rep) {
    n = n_;
    curr.resize(0);
    curr.resize(n, false);
    best = curr;
    best_duration.value = 0;
    data.resize(n);
    double flip_p = 0.5;
    double shift_p = 0.5;
    for (size_t i = 0; i < rep; ++i) {
      mutate_flip(flip_p);
      if (!try_run()) flip_p /= 1.005;
      mutate_shift(shift_p);
      if (!try_run()) shift_p /= 1.005;
    }
    for (size_t i = 0; i < n; ++i) curr[i] = i % 2 == 0;
    try_run();
  }
};

template<auto... impls>
void blah() {
  std::tuple<state<impls>...> st;
  auto&& [... s] = st;
  (s.find_worst_case(10'000, 10'000), ...);
  auto bestt = std::tuple{s.best...};
  auto&& [... best] = bestt;
  std::vector<bool> curr;
  ((curr = best, ((s.curr = curr, s.try_run()), ...)), ...);
  static constexpr auto foot = std::tuple{identifier_of(impls)...};
  static constexpr auto [... foo] = foot;
  (ivl::fmt::println("{} -> {}", foo, s.best_duration.value), ...);
}

/*
monotonic_contiguous_join_v1 -> 335064
monotonic_contiguous_join_v2 -> 362684
monotonic_contiguous_join_v5 -> 1355674
monotonic_contiguous_join_v6 -> 599012
 */

int ivl_main() {
  blah<
    ^^ivl::algorithm::monotonic_contiguous_join_v1, ^^ivl::algorithm::monotonic_contiguous_join_v2,
    ^^ivl::algorithm::monotonic_contiguous_join_v5, ^^ivl::algorithm::monotonic_contiguous_join_v6>();
  return 0;
}
