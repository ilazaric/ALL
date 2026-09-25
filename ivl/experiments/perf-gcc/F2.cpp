#include <array>
#include <cstdint>

// IVL disable_ivl_main_handler()

struct Mint {
  std::array<uint32_t, 1> data;

  friend constexpr Mint operator*(const Mint& a, const Mint& b) {
    return Mint{a.data[0] + b.data[0]};
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
IVL: constexpr_ops_count: 2960625
ver == 15.1.0
IVL: constexpr_ops_count: 4560945
ver == 16.1.0
IVL: constexpr_ops_count: 4560945
 */
