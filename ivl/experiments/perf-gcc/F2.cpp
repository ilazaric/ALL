#include <algorithm>
#include <functional>
#include <numeric>
#include <utility>
#include <cstdint>

// IVL disable_ivl_main_handler()

template<std::uint32_t... Mods>
struct MultiMint {
  static_assert((true && ... && (std::cmp_less(0, Mods) && std::cmp_less(Mods, 1ull << 31))));
  static_assert([] {
    std::array<std::uint32_t, sizeof...(Mods)> arr{Mods...};
    std::ranges::sort(arr);
    // for (auto idx : std::views::iota(1u, arr.size()))
    for (uint32_t idx =1u; idx < arr.size(); ++idx)
      if (arr[idx] == arr[idx - 1]) return false;
    return true;
  }());

  static constexpr std::array<std::uint32_t, sizeof...(Mods)> ModsArray{Mods...};

  template<std::uint32_t arg>
  static constexpr std::uint32_t ModIndex = std::distance(ModsArray.begin(), std::ranges::find(ModsArray, arg));

  std::array<std::uint32_t, sizeof...(Mods)> data;

  constexpr MultiMint() : data{} {}

  constexpr MultiMint(std::integral auto arg)
      : data{static_cast<std::uint32_t>(arg % Mods < 0 ? arg % Mods + Mods : arg % Mods)...} {}

  constexpr std::uint32_t& operator[](std::uint32_t idx) { return data[idx]; }
  // TODO: should this return value, not cref?
  constexpr const std::uint32_t& operator[](std::uint32_t idx) const { return data[idx]; }

  friend constexpr MultiMint<Mods...> operator*(const MultiMint& a, const MultiMint& b) {
    return MultiMint::unsafe_create({(static_cast<std::uint32_t>(
      static_cast<std::uint64_t>(a[ModIndex<Mods>]) * static_cast<std::uint64_t>(b[ModIndex<Mods>]) % Mods
    ))...});
  }

  static constexpr MultiMint unsafe_create(std::array<std::uint32_t, sizeof...(Mods)> arg) {
    MultiMint out;
    out.data = arg;
    return out;
  }
};

constexpr std::uint32_t Mod = 998'244'353;
using Mint = MultiMint<Mod>;

// TODO: this takes 35s to compile
auto factorials_storage = [] {
  std::array<Mint, 20'005> out{};
  out[0] = 1;
  // for (auto i : std::views::iota(1u, out.size()))
  for (uint32_t i = 1u; i < out.size(); ++i)
    out[i] = out[i - 1] * i;
  return out;
}();
