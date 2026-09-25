#include <cstdint>
#include <cstddef>
#include <debug/assertions.h>

// IVL disable_ivl_main_handler()

template<typename _Tp, std::size_t _Nm>
struct array {
  _Tp _M_elems[_Nm];

  [[__gnu__::__const__, __gnu__::__always_inline__]]
  constexpr size_t
  size() const noexcept { return _Nm; }

  constexpr _Tp
  operator[](size_t __n) const noexcept
  {
    __glibcxx_requires_subscript(__n);
    return _M_elems[__n];
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
IVL: constexpr_ops_count: 2080472
ver == 15.1.0
IVL: constexpr_ops_count: 3680812
ver == 16.1.0
IVL: constexpr_ops_count: 3680812
 */
