#include <ivl/format>
#include <ivl/reflection/test_attribute>
#include "default"
#include <set>
#include <vector>

// IVL test_only()

template<typename T>
void flatten_into(const ivl::avl::node<T>& t, std::vector<T>& out) {
  if (!t) return;
  flatten_into(t->left, out);
  out.push_back(t->value);
  flatten_into(t->right, out);
}

template<typename T>
std::vector<T> flatten(const ivl::avl::tree<T>& t) {
  std::vector<T> ret;
  flatten_into(t.root, ret);
  return ret;
}

bool test(const std::vector<int>& inserts, bool loud = true) {
  std::set<int> expected;
  ivl::avl::tree<int> actual;
  for (auto i : inserts) {
    expected.insert(i);
    actual.insert(i);
  }
  std::vector expected_vec(std::from_range, expected);
  auto actual_vec = flatten(actual);
  bool order = expected_vec == actual_vec;
  bool inv = check_invariant(actual.root, -1);
  if (order && inv) return true;
  if (!loud) return false;
  ivl::fmt::println(stderr, "FAIL ON inserts={}", inserts);
  return false;
}

bool test_wide(size_t n, size_t m, size_t k) {
  srand(1337);
  size_t last_fail_size = -1;
  size_t fail_count = 0;
  for (size_t i = 0; i < n; ++i) {
    std::vector<int> a(rand() % m);
    for (auto& e : a) e = rand() % k;
    bool smaller = a.size() < last_fail_size;
    if (test(a, smaller)) continue;
    ++fail_count;
    if (!smaller) continue;
    last_fail_size = a.size();
  }
  if (fail_count == 0) return true;
  ivl::fmt::println(stderr, "seen {}/{} == {:.2f}% failures", fail_count, n, (double)fail_count / n * 100);
  return false;
}

[[= ivl::test]] void test_wide_1() { contract_assert(test_wide(200000, 100, 50)); }

[[= ivl::test]] void test_wide_2() { contract_assert(test_wide(20000, 1000, 1000)); }
