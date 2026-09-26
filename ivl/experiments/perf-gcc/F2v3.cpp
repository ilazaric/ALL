#include <bits/c++config.h>

// IVL disable_ivl_main_handler()
// IVL add_compiler_flags("-fconstexpr-ops-limit=1000000000000")

#define __glibcxx_requires_subscript(_N) __glibcxx_assert(_N < this->size())

template<typename T, unsigned N>
struct array {
  T elems[N];

  constexpr unsigned size() const { return N; }

  constexpr T& operator[](unsigned n) {
    __glibcxx_requires_subscript(n);
    return elems[n];
  }
};

constexpr unsigned N = 400;

constexpr auto storage = [] {
  array<array<unsigned, N>, N> out{};
  out[0][0] = 1;
  for (unsigned i = 1; i < N; ++i) out[i][0] = out[0][i] = 1;
  for (unsigned i = 1; i < N; ++i)
    for (unsigned j = 1; j < N; ++j) out[i][j] = out[i - 1][j] + out[i][j - 1];
  return out;
}();
