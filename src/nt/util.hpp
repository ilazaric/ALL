#pragma once

#include <concepts>
#include <cassert>
#include <span>
#include <vector>
#include <utility>
#include <ranges>
#include <functional>

#include <ivl/literals/ints.hpp>
using namespace ivl::literals::ints_exact;

namespace ivl::nt {

  // TODO: template stuff

  // the slow stupid form
  bool is_prime(std::uint64_t n){
    if (n <= 1)
      return false;
    for (std::uint64_t p = 2; p*p <= n; ++p)
      if (n % p == 0)
        return false;
    return true;
  }

  template<typename T, typename U = std::multiplies<T>>
  constexpr T power(T a, std::uint64_t e, U&& mul = U{}){
    if (e == 0)
      return T{1};
    // trying something new
    T r{a};
    for (auto idx : std::views::iota(0, 63-std::countl_zero(e)) | std::views::reverse){
      r = mul(r, r);
      if (e & (1 << idx))
        r = mul(r, a);
    }
    return r;
  }

  template<std::uint32_t Mod = 0_u32>
  struct multiplies {
    constexpr std::uint32_t operator()(std::uint32_t a, std::uint32_t b) const {
      return (std::uint32_t)((std::uint64_t)a * b % Mod);
    }
  };
  
  template<>
  struct multiplies<0_u32> {
    std::uint32_t Mod;

    constexpr multiplies(std::uint32_t Mod) : Mod(Mod){}

    constexpr multiplies(const multiplies&) = default;
    constexpr multiplies(multiplies&&) = default;

    // feels right, maybe change dunno
    constexpr multiplies& operator=(const multiplies&) = delete;
    constexpr multiplies& operator=(multiplies&&) = delete;

    constexpr ~multiplies() = default;

    constexpr std::uint32_t operator()(std::uint32_t a, std::uint32_t b) const {
      return (std::uint32_t)((std::uint64_t)a * (std::uint64_t)b % Mod);
    }
  };

  // not sure if needed
  template<typename T>
  multiplies(T&&) -> multiplies<0_u32>;

  constexpr std::uint64_t gcd(std::uint64_t a, std::uint64_t b){
    if (a == 0 || b == 0)
      return a+b;
    
    int final_shift = std::min(std::countr_zero(a), std::countr_zero(b));
    a >>= std::countr_zero(a);
    b >>= std::countr_zero(b);

    while (true){
      a %= b;
      if (a == 0)
        return b << final_shift;
      a >>= std::countr_zero(a);
      b %= a;
      if (b == 0)
        return a << final_shift;
      b >>= std::countr_zero(a);
    }
  }

  constexpr std::pair<std::int64_t, std::int64_t> extended_euclid(std::uint32_t a, std::uint32_t b){
    if (a == 0) return {0, 1};
    if (b == 0) return {1, 0};

    std::pair<std::int64_t, std::int64_t> ap{1, 0};
    std::pair<std::int64_t, std::int64_t> bp{0, 1};

    while (b){
      auto ratio = a / b;
      a -= b * ratio;
      ap.first -= bp.first * ratio;
      ap.second -= bp.second * ratio;
      std::swap(a, b);
      std::swap(ap, bp);
    }
    
    return ap;
  }
  
  // alt strategy would be a**(phi(m)-1)
  constexpr std::uint32_t modular_inverse(std::uint32_t a, std::uint32_t m){
    assert(gcd(a, m) == 1);

    auto [ac, mc] = extended_euclid(a, m);
    ac %= m;
    return ac < 0 ? ac+m : ac;
  }

  struct GarnerVector : std::vector<std::uint32_t> {
    using std::vector<std::uint32_t>::vector;
  };

  struct GarnerTable {
    std::vector<std::uint32_t> mods;
    std::vector<std::vector<std::uint32_t>> invs;

    GarnerTable(std::span<const std::uint32_t> arg){
      mods.reserve(arg.size());
      for (auto mod : arg)
        mods.push_back(mod);

      invs.resize(arg.size());
      for (auto i : std::views::iota(0_u32, arg.size()))
        for (auto j : std::views::iota(0_u32, i))
          invs[i].push_back(modular_inverse(arg[j], arg[i]));
    }

    GarnerVector convert_representation(const std::uint32_t* in) const {
      GarnerVector out(mods.size());

      for (auto i : std::views::iota(0_u32, mods.size())){
        std::uint64_t val = in[i];
        for (auto j : std::views::iota(0_u32, i))
          val = (val - out[j] + mods[i]) * invs[i][j] % mods[i];
        out[i] = (std::uint32_t)val;
      }

      return out;
    }

    template<typename T>
    void apply_inplace(const GarnerVector& gv, T& out) const {
      out += gv.back();
      for (auto idx : std::views::iota(0_u32, gv.size()-1) | std::views::reverse){
        out *= mods[idx];
        out += gv[idx];
      }
    }

    template<typename T>
    T apply(const GarnerVector& gv) const {
      T out{};
      apply_inplace(gv, out);
      return out;
    }
  };

  // std::vector<std::uint32_t> garner(const std::uint32_t* in, const GarnerStuff& gs){
  //   std::vector<std::uint32_t> out(gs.mods.size());

  //   for (auto i : std::views::iota(0_u32, gs.mods.size())){
  //     std::uint64_t val = in[i];
  //     for (auto j : std::views::iota(0_u32, i))
  //       val = (val - out[j] + gs.mods[i]) * gs.invs[i][j] % gs.mods[i];
  //     out[i] = (std::uint32_t)val;
  //   }

  //   return out;
  // }

  // template<typename T>
  // T garner_apply(const std::vector<std::uint32_t>& g, const GarnerStuff& gs){
  //   T out{g.back()};
  //   for (auto idx : std::views::iota(0_u32, g.size()-1) | std::views::reverse)
  //     out = out * gs.mods[idx] + g[idx];
  //   return out;
  // }
  
} // namespace ivl::nt
