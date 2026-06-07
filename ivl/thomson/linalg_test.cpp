#include <ivl/reflection/test_attribute>
#include "linalg"
#include <cmath>
#include <iostream>

// IVL test_only()

[[= ivl::test]] void test_fib() {
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
