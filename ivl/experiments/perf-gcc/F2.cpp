#include <debug/assertions.h>
#include <cstddef>
#include <cstdint>

// IVL disable_ivl_main_handler()

template<typename T, size_t N>
struct array {
  T _M_elems[N];

  constexpr size_t size() const noexcept { return N; }

  constexpr T operator[](size_t n) const noexcept {
    __glibcxx_requires_subscript(n);
    return _M_elems[n];
  }
};

struct Mint {
  array<uint32_t, 1> data;
};

auto factorials_storage = [] {
  array<Mint, 20'005> out{};
  out[0] = {1};
  for (uint32_t i = 1u; i < out.size(); ++i) out[i] = Mint{out[i - 1].data[0] * Mint{i}.data[0]};
  return out;
}();

/*
ver == 14.4.0
IVL: constexpr_ops_count: 2100477
ver == 15.1.0
IVL: constexpr_ops_count: 3700817
ver == 16.1.0
IVL: constexpr_ops_count: 3700817
 */
