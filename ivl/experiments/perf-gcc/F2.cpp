#include <array>
#include <cstdint>

// IVL disable_ivl_main_handler()

struct Mint {
  std::array<uint32_t, 1> data;

  constexpr uint32_t operator[](uint32_t idx) const { return data[idx]; }

  friend constexpr Mint operator*(const Mint& a, const Mint& b) {
    return Mint{.data{a[0] + b[0]}};
  }
};

auto factorials_storage = [] {
  std::array<Mint, 20'005> out{};
  out[0] = 1;
  for (uint32_t i = 1u; i < out.size(); ++i) out[i] = out[i - 1] * Mint{i};
  return out;
}();

/*
ver == 14.4.0
IVL: constexpr_ops_count: 3400713
ver == 15.1.0
IVL: constexpr_ops_count: 5001033
ver == 16.1.0
IVL: constexpr_ops_count: 5001033
 */
