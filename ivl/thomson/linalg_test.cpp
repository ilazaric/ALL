#include <ivl/reflection/test_attribute>
#include "linalg"
#include <cmath>
#include <iostream>
#include <random>

// IVL test_only()

[[= ivl::test]] void test_fib1() {
  mdarray m(shape_t::linear_operator(shape_t::vector(2), shape_t::vector(2)));
  m[1][1] = m[1][0] = m[0][1] = 1;
  mdarray v(shape_t::vector(2));
  v[1] = 1;
  for (int i = 0; i < 10; ++i) v = compose(m, v);
  contract_assert((int)std::round(v[0].extract_number()) == 55);
  contract_assert((int)std::round(v[1].extract_number()) == 89);
}

[[= ivl::test]] void test_fib2() {
  mdarray m(shape_t::linear_operator(shape_t::vector(2), shape_t::vector(2)));
  m[1][1] = m[1][0] = m[0][1] = 1;
  mdarray n(shape_t::linear_operator(shape_t::vector(2), shape_t::vector(2)));
  n[0][0] = n[1][1] = 1;
  for (int i = 0; i < 10; ++i) n = compose(m, n);
  contract_assert((int)std::round(n[0][0].extract_number()) == 34);
  contract_assert((int)std::round(n[0][1].extract_number()) == 55);
  contract_assert((int)std::round(n[1][0].extract_number()) == 55);
  contract_assert((int)std::round(n[1][1].extract_number()) == 89);
}

[[= ivl::test]] void test_fib3() {
  mdarray m(shape_t::linear_operator(shape_t::vector(2), shape_t::vector(2)));
  m[1][1] = m[1][0] = m[0][1] = 1;
  mdarray v(shape_t::vector(2));
  v[1] = 1;
  for (int i = 0; i < 10; ++i) v = m(v);
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
  mdarray getmd(double lo, double hi, const shape_t& shape) {
    mdarray ret(shape);
    for (auto& el : ret.data) el = get(lo, hi);
    return ret;
  }
  mdarray getmd(double x, const shape_t& shape) { return getmd(-x, x, shape); }
  mdarray getmd(const shape_t& shape) { return getmd(1, shape); }
};

[[= ivl::test]] void test_tensor1() {
  rnd_gen g(1337);
  auto m = g.getmd(10, shape_t::linear_operator(shape_t::vector(3), shape_t::vector(4), shape_t::vector(5)));
  auto v1 = g.getmd(shape_t::vector(3));
  auto v2 = g.getmd(shape_t::vector(4));
  auto v3 = g.getmd(shape_t::vector(5));
  auto a = m(v1)(v2)(v3).extract_number();
  auto b = compose(v3, compose(v2, compose(v1, m))).extract_number();
  std::cerr << a << std::endl;
  std::cerr << b << std::endl;
  contract_assert(abs(a - b) < 1e-5);
}

[[= ivl::test]] void test_tensor2() {
  rnd_gen g(4010101);
  auto a = g.getmd(2, shape_t::linear_operator(shape_t::vector(3), shape_t::vector(4), shape_t::vector(5)));
  auto b = g.getmd(2, shape_t::linear_operator(shape_t::vector(5), shape_t::vector(4), shape_t::vector(3)));
  auto c = compose(a, b);
  contract_assert(c.shape.linear_operator_rank() == 3);
  contract_assert(
    c.shape == shape_t::linear_operator(shape_t::vector(3), shape_t::vector(4), shape_t::vector(4), shape_t::vector(3))
  );
  auto v1 = g.getmd(2, shape_t::vector(3));
  auto v2 = g.getmd(2, shape_t::vector(4));
  auto v3 = g.getmd(2, shape_t::vector(4));
  auto v4 = g.getmd(2, shape_t::vector(3));
  auto x = b(a(v1)(v2))(v3)(v4).extract_number();
  auto y = c(v1)(v2)(v3)(v4).extract_number();
  std::cerr << x << std::endl;
  std::cerr << y << std::endl;
  contract_assert(abs(x - y) < 1e-5);
}
