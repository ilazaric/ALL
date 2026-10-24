#include <ivl/format>
#include <ivl/reflection/test_attribute>
#include "brute"
#include "default"
#include <cassert>

// IVL test_only()

bool compare(std::vector<std::pair<int, int>> a, std::vector<std::pair<int, int>> b, bool loud = true) {
  auto a1 = a;
  auto a2 = a;
  auto order = [](const auto& l, const auto& r) { return l.first <=> r.first; };
  auto transform_or_skip = [](auto& l, const auto& r) {
    using CR = ivl::algorithm::monotonic_vector_update_collision_resolution;
    l.second += r.second;
    return l.second == 0 ? CR::erase : CR::preserve;
  };
  ivl::algorithm::monotonic_vector_update_brute(a1, b, order, transform_or_skip);
  ivl::algorithm::monotonic_vector_update(a2, b, order, transform_or_skip);
  if (a1 == a2) return true;
  if (loud) {
    ivl::fmt::println(stderr, "FAIL ON a={}\n", a);
    ivl::fmt::println(stderr, "        b={}\n", b);
    ivl::fmt::println(stderr, "       brute={}\n", a1);
    ivl::fmt::println(stderr, "        algo={}\n\n", a2);
  }
  return false;
}

[[= ivl::test]] void compare_test_1() { assert(compare({{10, -1}, {13, -2}}, {{4, -4}, {13, 8}})); }

[[= ivl::test]] void compare_test_wide() {
  srand(1337);
  size_t last_fail_size = -1;
  size_t fail_count = 0;
  size_t n = 1000000;
  for (size_t i = 0; i < n; ++i) {
    std::vector<std::pair<int, int>> a(rand() % 20);
    std::vector<std::pair<int, int>> b(rand() % 20);
    for (auto& [k, v] : a) k = rand() % 10 + 1, v = (rand() % 10 + 1) * (rand() % 2 * 2 - 1);
    for (auto& [k, v] : b) k = rand() % 10 + 1, v = (rand() % 10 + 1) * (rand() % 2 * 2 - 1);
    for (size_t j = 1; j < a.size(); ++j) a[j].first += a[j - 1].first;
    for (size_t j = 1; j < b.size(); ++j) b[j].first += b[j - 1].first;
    bool smaller = a.size() + b.size() < last_fail_size;
    if (compare(a, b, smaller)) continue;
    ++fail_count;
    if (!smaller) continue;
    last_fail_size = a.size() + b.size();
  }
  if (fail_count == 0) return;
  ivl::fmt::println(stderr, "seen {}/{} == {:.2f}% failures", fail_count, n, (double)fail_count / n * 100);
  exit(1);
}
