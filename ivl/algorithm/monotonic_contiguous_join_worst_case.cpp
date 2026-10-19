#include <ivl/command_line_argument_parsing/implicit_exposed>
#include <ivl/format>
#include <ivl/metaheuristics/pseudo_boolean>
#include <ivl/utility/tsc>
#include <ivl/utility/tuple_broadcast>
#include "monotonic_contiguous_join"
#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>

template<auto impl>
struct runner {
  std::vector<size_t> data;
  size_t count = 0;
  size_t sum = 0;

  runner(size_t n) : data(n, 0) {}

  [[gnu::noinline]]
  void do_run(auto... args) {
    [:impl:](args...);
    asm volatile("" ::"m"(data));
  }

  double run(std::span<const bool> curr) {
    contract_assert(curr.size() <= data.size());
    ++count;
    size_t n = curr.size();
    sum += n;
    size_t foo = 0;
    size_t bar = 0;
    for (size_t i = 0; i < n; ++i) {
      if (curr[i]) data[foo++] = i;
      else data[n - 1 - (bar++)] = i;
    }
    std::reverse(data.begin() + foo, data.begin() + n);
    auto start = ivl::tsc_now();
    do_run(data.begin(), data.begin() + foo, data.begin() + n);
    auto end = ivl::tsc_now();
    return (double)((end - start).value);
  }

  auto operator()(std::span<const bool> curr) { return run(curr); }
};

template<auto impl>
struct state {
  runner<impl> r;
  ivl::pseudo_boolean_optimize_t o;

  state(size_t n, size_t rep) : r(n), o{n, r} {
    o.incremental_init_search();
    contract_assert(o.best.size() == n);
    contract_assert(o.curr.size() == n);
    ivl::fmt::println("complex_search(): {}", o.complex_search(rep));
    ivl::fmt::println("single_flip_search(): {}", o.single_flip_search());
    ivl::fmt::println("repeat_flip_search(): {}", o.repeat_flip_search());
  }

  void dump() {
    static constexpr auto foo = identifier_of(impl);
    ivl::fmt::println("{} -> {}", foo, o.best_eval);
    if (implicit_flag("dump_stats")) {
      ivl::fmt::println("run count: {}", r.count);
      ivl::fmt::println("run sum: {}", r.sum);
    }
    if (!implicit_flag("dump_opts")) return;
    size_t cnt = 0;
    bool kind = o.best[0];
    for (auto&& el : o.best) {
      if (kind == el) ++cnt;
      else {
        ivl::fmt::print("{}{} ", cnt, kind ? 'A' : 'B');
        cnt = 1;
        kind = !kind;
      }
    }
    ivl::fmt::println("{}{}\n", cnt, kind ? 'A' : 'B');
  }
};

template<auto... impls>
void blah() {
  auto [... s] = ivl::tuple_broadcast_values<impls...>([]<auto impl> {
    return state<impl>(implicit_value("n", 10'000UZ), implicit_value("rep", 10'000UZ));
  });
  // auto [... s] = states<impls...>(implicit_value("n", 10'000UZ), implicit_value("rep", 10'000UZ));
  auto bestt = std::tuple{s.o.best...};
  auto&& [... best] = bestt;
  std::vector<bool> curr;
  ((curr = best, ((s.o.curr = curr, s.o.try_run()), ...)), ...);
  ((s.dump()), ...);
}

/*
monotonic_contiguous_join_v1 -> 300820
monotonic_contiguous_join_v2 -> 387768
monotonic_contiguous_join_v5 -> 1345530
monotonic_contiguous_join_v6 -> 602110

monotonic_contiguous_join_v1 -> 649004
monotonic_contiguous_join_v2 -> 290756
monotonic_contiguous_join_v5 -> 1182552
monotonic_contiguous_join_v6 -> 696630

monotonic_contiguous_join_v1 -> 651002
monotonic_contiguous_join_v2 -> 299416
monotonic_contiguous_join_v5 -> 1347812
monotonic_contiguous_join_v6 -> 615150
*/

int ivl_main(ivl::cmdline_parsing::implicit) {
  blah<
    ^^ivl::algorithm::monotonic_contiguous_join_v1, ^^ivl::algorithm::monotonic_contiguous_join_v2,
    ^^ivl::algorithm::monotonic_contiguous_join_v5, ^^ivl::algorithm::monotonic_contiguous_join_v6>();
  return 0;
}
