#include <ivl/format>
#include <ivl/reflection/test_attribute>
#include <ivl/reflection/test_matrix>
#include "all"
#include <algorithm>
#include <vector>

// IVL test_only()

struct S {
  int x;
  std::strong_ordering operator<=>(const S&) = delete;
};

template<auto impl>
struct bundle {
  bool test(const std::vector<S>& orig, size_t m, bool loud = true) {
    auto actual = orig;
    auto expected = orig;
    auto gt = [](const S& a, const S& b) { return a.x > b.x; };
    auto cmp3 = [](const S& a, const S& b) { return b.x <=> a.x; };
    std::ranges::sort(expected, gt);
    [:impl:](actual.begin(), actual.begin() + m, actual.end(), cmp3);
    if (std::ranges::equal(actual, expected, {}, &S::x, &S::x)) return true;
    if (loud) {
      ivl::fmt::println(stderr, "FAIL ON v={}", orig | std::views::transform(&S::x));
      ivl::fmt::println(stderr, "        m={}", m);
      ivl::fmt::println(stderr, "   actual={}", actual | std::views::transform(&S::x));
      ivl::fmt::println(stderr, " expected={}\n", expected | std::views::transform(&S::x));
    }
    return false;
  }

  [[= ivl::test]] void test_wide() {
    srand(1337);
    size_t last_fail_size = -1;
    size_t fail_count = 0;
    size_t n = 1000000;
    for (size_t i = 0; i < n; ++i) {
      std::vector<S> a(rand() % 100);
      for (auto& e : a) e.x = rand() % 100;
      auto m = rand() % (a.size() + 1);
      auto gt = [](const S& a, const S& b) { return a.x > b.x; };
      std::sort(a.begin(), a.begin() + m, gt);
      std::sort(a.begin() + m, a.end(), gt);
      bool smaller = a.size() < last_fail_size;
      if (test(a, m, smaller)) continue;
      ++fail_count;
      if (!smaller) continue;
      last_fail_size = a.size();
    }
    if (fail_count == 0) return;
    ivl::fmt::println(stderr, "seen {}/{} == {:.2f}% failures", fail_count, n, (double)fail_count / n * 100);
    exit(1);
  }
};

constexpr ivl::test_matrix impls{
  ^^ivl::algorithm::monotonic_join_v1,
  ^^ivl::algorithm::monotonic_join_v2,
  ^^ivl::algorithm::monotonic_join_v5,
  ^^ivl::algorithm::monotonic_join_v6,
};

constexpr ivl::test_broadcast bundles(^^bundle, impls);
