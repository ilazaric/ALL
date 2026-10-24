#include <ivl/format>
#include <ivl/reflection/test_attribute>
#include "brute"
#include "default"
#include <cassert>

// IVL test_only()

bool run(std::vector<std::pair<int, int>> a, std::vector<std::pair<int, int>> b, bool loud = true) {
  auto a1 = a;
  auto a2 = a;
  auto order = [](const auto& l, const auto& r) { return l.first <=> r.first; };
  auto hook = [](auto& l, const auto& r) {
    if (!l) l.emplace(r->first, 0);
    if (r) l->second += r->second;
    if (l->second == 0) l.reset();
  };
  ivl::algorithm::monotonic_vector_update_brute(a1, b, order, hook);
  ivl::algorithm::monotonic_vector_update(a2, b, order, hook);
  if (a1 == a2 && std::ranges::count(a1, 0, &std::pair<int, int>::second) == 0) return true;
  if (loud) {
    ivl::fmt::println(stderr, "FAIL ON a={}", a);
    ivl::fmt::println(stderr, "        b={}", b);
    ivl::fmt::println(stderr, "       brute={}", a1);
    ivl::fmt::println(stderr, "        algo={}\n", a2);
  }
  return false;
}

[[= ivl::test]] void test_1() { assert(run({{10, -1}, {13, -2}}, {{4, -4}, {13, 8}})); }

struct input {
  std::vector<std::pair<int, int>> a;
  std::vector<std::pair<int, int>> b;
};

std::vector<input> wide_inputs() {
  srand(1337);
  size_t n = 100000;
  std::vector<input> ret;
  ret.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    std::vector<std::pair<int, int>> a(rand() % 20);
    std::vector<std::pair<int, int>> b(rand() % 20);
    for (auto& [k, v] : a) k = rand() % 10 + 1, v = (rand() % 10 + 1) * (rand() % 2 * 2 - 1);
    for (auto& [k, v] : b) k = rand() % 10 + 1, v = (rand() % 10 + 1) * (rand() % 2 * 2 - 1);
    for (size_t j = 1; j < a.size(); ++j) a[j].first += a[j - 1].first;
    for (size_t j = 1; j < b.size(); ++j) b[j].first += b[j - 1].first;
    ret.emplace_back(std::move(a), std::move(b));
  }
  return ret;
}

[[= ivl::test]] void test_wide() {
  size_t last_fail_size = -1;
  size_t fail_count = 0;
  size_t count = 0;
  for (auto&& [a, b] : wide_inputs()) {
    ++count;
    bool smaller = a.size() + b.size() < last_fail_size;
    if (run(a, b, smaller)) continue;
    ++fail_count;
    if (smaller) last_fail_size = a.size() + b.size();
  }
  if (fail_count == 0) return;
  ivl::fmt::println(stderr, "seen {}/{} == {:.2f}% failures", fail_count, count, (double)fail_count / count * 100);
  exit(1);
}

bool run_2(std::vector<std::pair<int, int>> a, std::vector<std::pair<int, int>> b, bool loud = true) {
  auto a1 = a;
  auto a2 = a;
  auto order = [](const auto& l, const auto& r) { return l.first <=> r.first; };
  auto hook = [](auto& l, const auto& r) {
    auto [k, v] = l ? *l : *r;
    if (l && r) v += r->second;
    if (l) v += 10;
    if (r) v += 20;
    l.emplace(k, v);
  };
  ivl::algorithm::monotonic_vector_update_brute(a1, b, order, hook);
  ivl::algorithm::monotonic_vector_update(a2, b, order, hook);
  if (a1 == a2) return true;
  if (loud) {
    ivl::fmt::println(stderr, "FAIL ON a={}", a);
    ivl::fmt::println(stderr, "        b={}", b);
    ivl::fmt::println(stderr, "       brute={}", a1);
    ivl::fmt::println(stderr, "        algo={}\n", a2);
  }
  return false;
}

[[= ivl::test]] void test_wide_2() {
  size_t last_fail_size = -1;
  size_t fail_count = 0;
  size_t count = 0;
  for (auto&& [a, b] : wide_inputs()) {
    ++count;
    bool smaller = a.size() + b.size() < last_fail_size;
    if (run_2(a, b, smaller)) continue;
    ++fail_count;
    if (smaller) last_fail_size = a.size() + b.size();
  }
  if (fail_count == 0) return;
  ivl::fmt::println(stderr, "seen {}/{} == {:.2f}% failures", fail_count, count, (double)fail_count / count * 100);
  exit(1);
}
