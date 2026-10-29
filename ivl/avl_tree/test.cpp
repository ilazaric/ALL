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

struct op {
  int value;
  bool erase;
};

bool test(const std::vector<op>& ops, bool loud = true) {
  std::set<int> expected;
  ivl::avl::tree<int> actual;
  for (auto [v, e] : ops) {
    if (e) {
      expected.erase(v);
      actual.erase(v);
    } else {
      expected.insert(v);
      actual.insert(v);
    }
  }
  std::vector expected_vec(std::from_range, expected);
  auto actual_vec = flatten(actual);
  bool order = expected_vec == actual_vec;
  bool inv = check_invariant(actual.root, -1);
  if (order && inv) return true;
  if (!loud) return false;
  ivl::fmt::println(stderr, "FAIL ON ops={::s}", std::views::transform(ops, [](op o) {
                      return std::format("{}{}", o.erase ? '-' : '+', o.value);
                    }));
  return false;
}

bool test_wide(size_t n, size_t m, size_t k, bool erases = false) {
  srand(1337);
  size_t last_fail_size = -1;
  size_t fail_count = 0;
  for (size_t i = 0; i < n; ++i) {
    std::vector<op> a(rand() % m);
    for (auto& [v, e] : a) {
      v = rand() % k;
      e = rand() % (erases + 1);
    }
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

[[= ivl::test]] void test_wide_3() { contract_assert(test_wide(200000, 100, 20, true)); }

[[= ivl::test]] void test_wide_4() { contract_assert(test_wide(20000, 1000, 300, true)); }
