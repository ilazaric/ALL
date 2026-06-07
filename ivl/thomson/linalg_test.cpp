#include <ivl/reflection/test_attribute>
#include "linalg"
#include <cmath>
#include <iostream>
#include <random>

// IVL test_only()

[[= ivl::test]] void test_fib1() {
  mdarray m(shaped, 2, 2);
  m[1][1] = m[1][0] = m[0][1] = 1;
  mdarray v(shaped, 2);
  v[1] = 1;
  for (int i = 0; i < 10; ++i) v = compose(m, v);
  contract_assert((int)std::round(v[0].extract_number()) == 55);
  contract_assert((int)std::round(v[1].extract_number()) == 89);
}

[[= ivl::test]] void test_fib2() {
  mdarray m(shaped, 2, 2);
  m[1][1] = m[1][0] = m[0][1] = 1;
  mdarray n(shaped, 2, 2);
  n[0][0] = n[1][1] = 1;
  for (int i = 0; i < 10; ++i) n = compose(m, n);
  contract_assert((int)std::round(n[0][0].extract_number()) == 34);
  contract_assert((int)std::round(n[0][1].extract_number()) == 55);
  contract_assert((int)std::round(n[1][0].extract_number()) == 55);
  contract_assert((int)std::round(n[1][1].extract_number()) == 89);
}

[[= ivl::test]] void test_fib3() {
  mdarray m(shaped, 2, 2);
  m[1][1] = m[1][0] = m[0][1] = 1;
  mdarray v(shaped, 2);
  v[1] = 1;
  for (int i = 0; i < 10; ++i) v = m(v.data);
  contract_assert((int)std::round(v[0].extract_number()) == 55);
  contract_assert((int)std::round(v[1].extract_number()) == 89);
}

struct rnd_gen {
  std::mt19937 gen;
  explicit rnd_gen(std::size_t seed) : gen(seed) {}
  double get(double lo, double hi) {
    contract_assert(lo < hi);
    return std::uniform_real_distribution<>(lo, hi)(gen);
  }
  double get(double x) { return get(-x, x); }
  double get() { return get(1); }
  mdarray getmd(double lo, double hi, shaped_t, auto... extents) {
    mdarray ret(shaped, extents...);
    for (auto& el : ret.data) el = get(lo, hi);
    return ret;
  }
  mdarray getmd(double x, shaped_t, auto... extents) { return getmd(-x, x, shaped, extents...); }
  mdarray getmd(shaped_t, auto... extents) { return getmd(1, shaped, extents...); }
};

[[= ivl::test]] void test_tensor1() {
  rnd_gen g(1337);
  auto m = g.getmd(10, shaped, 3, 4, 5);
  auto v1 = g.getmd(shaped, 3);
  auto v2 = g.getmd(shaped, 4);
  auto v3 = g.getmd(shaped, 5);
  auto a = m(v1.data)(v2.data)(v3.data).extract_number();
  auto b = compose(v3, compose(v2, compose(v1, m))).extract_number();
  std::cerr << a << std::endl;
  std::cerr << b << std::endl;
  contract_assert(abs(a - b) < 1e-5);
}

[[= ivl::test]] void test_tensor2() {
  rnd_gen g(4010101);
  auto a = g.getmd(2, shaped, 3, 4, 5);
  auto b = g.getmd(2, shaped, 5, 4, 3);
  auto c = compose(a, b);
  contract_assert(c.rank() == 4);
  contract_assert(c.shape == std::vector<std::size_t>{3, 4, 4, 3});
  auto v1 = g.getmd(2, shaped, 3);
  auto v2 = g.getmd(2, shaped, 4);
  auto v3 = g.getmd(2, shaped, 4);
  auto v4 = g.getmd(2, shaped, 3);
  auto x = b(a(v1.data)(v2.data).data)(v3.data)(v4.data).extract_number();
  auto y = c(v1.data)(v2.data)(v3.data)(v4.data).extract_number();
  std::cerr << x << std::endl;
  std::cerr << y << std::endl;
  contract_assert(abs(x - y) < 1e-5);
}
