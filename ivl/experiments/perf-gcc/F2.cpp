#include <array>
#include <cstdint>

// IVL disable_ivl_main_handler()

struct Mint {
  std::array<uint32_t, 1> data;
};

auto factorials_storage = [] {
  std::array<Mint, 20'005> out{};
  out[0] = 1;
  for (uint32_t i = 1u; i < out.size(); ++i) out[i] = Mint{out[i - 1].data[0] * Mint{i}.data[0]};
  return out;
}();

/*
ver == 14.4.0
IVL: constexpr_ops_count: 2620557
ver == 15.1.0
IVL: constexpr_ops_count: 4220877
ver == 16.1.0
IVL: constexpr_ops_count: 4220877
 */
