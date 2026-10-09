#include <ivl/format>
#include <ivl/reflection/test_attribute>
#include <ivl/reflection/test_matrix>
#include "monotonic_contiguous_join"
#include <algorithm>
#include <cassert>
#include <ranges>

// IVL test_only()

template<auto impl>
struct bundle {
  bool test(const std::vector<int>& orig, size_t m, bool loud = true) {
    auto actual = orig;
    auto expected = orig;
    std::ranges::sort(expected);
    [:impl:](actual.begin(), actual.begin() + m, actual.end());
    if (actual == expected) return true;
    if (loud) {
      ivl::fmt::println(stderr, "FAIL ON v={}", orig);
      ivl::fmt::println(stderr, "        m={}", m);
      ivl::fmt::println(stderr, "   actual={}", actual);
      ivl::fmt::println(stderr, " expected={}\n", expected);
    }
    return false;
  }

  [[= ivl::test]] void test_1() { assert(test({16, 58, 62, 80, 1, 8, 25}, 4)); }

  [[= ivl::test]] void test_2() { assert(test({20, 77, 80, 92, 1, 14, 54, 67}, 4)); }

  [[= ivl::test]] void test_3() { assert(test({12, 32, 56, 61, 83, 98, 0, 6, 17, 68}, 6)); }

  [[= ivl::test]] void test_4() { assert(test({35, 70, 82, 89, 96, 1, 3, 12, 65}, 5)); }

  [[= ivl::test]] void test_5() { assert(test({95, 12, 85}, 1)); }

  [[= ivl::test]] void test_wide() {
    srand(1337);
    size_t last_fail_size = -1;
    size_t fail_count = 0;
    size_t n = 1000000;
    for (size_t i = 0; i < n; ++i) {
      std::vector<int> a(rand() % 100);
      for (auto& e : a) e = rand() % 100;
      auto m = rand() % (a.size() + 1);
      std::sort(a.begin(), a.begin() + m);
      std::sort(a.begin() + m, a.end());
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
  ^^ivl::algorithm::monotonic_contiguous_join_v1, ^^ivl::algorithm::monotonic_contiguous_join_v2
};

constexpr ivl::test_broadcast bundles(^^bundle, impls);
