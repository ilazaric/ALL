#include <functional>
#include <numeric>
#include <utility>
#include <cstdint>

// IVL disable_ivl_main_handler()

constexpr uint32_t Mod = 998'244'353;

struct MultiMint {
  std::array<uint32_t, 1> data;

  constexpr MultiMint() : data{} {}

  constexpr MultiMint(std::integral auto arg)
      : data{static_cast<uint32_t>(arg % Mod < 0 ? arg % Mod + Mod : arg % Mod)} {}

  constexpr uint32_t& operator[](uint32_t idx) { return data[idx]; }
  constexpr const uint32_t& operator[](uint32_t idx) const { return data[idx]; }

  friend constexpr MultiMint operator*(const MultiMint& a, const MultiMint& b) {
    return MultiMint::unsafe_create({(static_cast<uint32_t>(
      static_cast<uint64_t>(a[0]) * static_cast<uint64_t>(b[0]) % Mod
    ))});
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
  for (uint32_t i = 1u; i < out.size(); ++i)
    out[i] = out[i - 1] * i;
  return out;
}();

/*
ver == 14.4.0
IVL: constexpr_ops_count: 4300943
ver == 15.1.0
IVL: constexpr_ops_count: 5901283
ver == 16.1.0
IVL: constexpr_ops_count: 5901283
 */
