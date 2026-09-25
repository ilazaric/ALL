#include <debug/assertions.h>

// IVL disable_ivl_main_handler()

template<typename T, unsigned N>
struct array {
  T elems[N];

  constexpr unsigned size() const { return N; }

  constexpr T& operator[](unsigned n) {
    __glibcxx_requires_subscript(n);
    return elems[n];
  }
};

constexpr unsigned N = 300;

constexpr auto storage = [] {
  array<array<unsigned, N>, N> out{};
  out[0][0] = 1;
  for (unsigned i = 1; i < N; ++i) out[i][0] = out[0][i] = 1;
  for (unsigned i = 1; i < N; ++i)
    for (unsigned j = 1; j < N; ++j) out[i][j] = out[i - 1][j] + out[i][j - 1];
  return out;
}();

/*
ver == 14.4.0
IVL: constexpr_ops_count: 1373401
ver == 15.1.0
IVL: constexpr_ops_count: 2202262
ver == 16.1.0
IVL: constexpr_ops_count: 2202262
 */
