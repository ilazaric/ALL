#include <debug/assertions.h>

// IVL disable_ivl_main_handler()
// IVL add_compiler_flags("-fconstexpr-ops-limit=1000000000000")

#define AT(arr, idx) arr[idx] // regular
// #define AT(arr, idx) arr.elems[idx] // inlined

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
  AT(AT(out, 0), 0) = 1;
  for (unsigned i = 1; i < N; ++i) AT(AT(out, i), 0) = AT(AT(out, 0), i) = 1;
  for (unsigned i = 1; i < N; ++i)
    for (unsigned j = 1; j < N; ++j) AT(AT(out, i), j) = AT(AT(out, i - 1), j) + AT(AT(out, i), j - 1);
  return out;
}();

/*
regular:
ver == 14.4.0
IVL: constexpr_ops_count: 22173301
ver == 15.1.0
IVL: constexpr_ops_count: 35568562
ver == 16.1.0
IVL: constexpr_ops_count: 35568562

inlined:
ver == 14.4.0
IVL: constexpr_ops_count: 5267633
ver == 15.1.0
IVL: constexpr_ops_count: 5267633
ver == 16.1.0
IVL: constexpr_ops_count: 5267633
 */
