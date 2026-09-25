#include <array>
#include <cstdint>

// IVL disable_ivl_main_handler()

struct MultiMint {
  std::array<uint32_t, 1> data;

  constexpr MultiMint() : data{} {}

  constexpr MultiMint(uint32_t arg) : data{arg} {}

  constexpr uint32_t operator[](uint32_t idx) const { return data[idx]; }

  friend constexpr MultiMint operator*(const MultiMint& a, const MultiMint& b) {
    return MultiMint::unsafe_create({(a[0] + b[0])});
  }

  static constexpr MultiMint unsafe_create(std::array<uint32_t, 1> arg) {
    MultiMint out;
    out.data = arg;
    return out;
  }
};

using Mint = MultiMint;

auto factorials_storage = [] {
  std::array<Mint, 20'005> out{};
  out[0] = 1;
  for (uint32_t i = 1u; i < out.size(); ++i) out[i] = out[i - 1] * i;
  return out;
}();

/*
ver == 14.4.0
IVL: constexpr_ops_count: 4020880
ver == 15.1.0
IVL: constexpr_ops_count: 5621220
ver == 16.1.0
IVL: constexpr_ops_count: 5621220
 */
