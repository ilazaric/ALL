#pragma once

#include <cstdint>
#include <array>
#include <algorithm> // TODO: figure out how to kill dep

#include "util.hpp"
#include <ivl/io/stlutils.hpp>

namespace ivl::nt {

// this should give me some out-of-order execution perf boosts
template<std::uint32_t ... Mods>
struct MultiMint {
  // `Mods` should be of reasonable size
  // `2*Mods` should fit in `std::uint32_t`
  // TODO: ^ this might be relaxable to `2*(Mods-1)`
  static_assert((true && ... && (std::cmp_less(0, Mods) && std::cmp_less(Mods, 1_u64 << 31))));
  // `Mods` should all be different
  // important for the `ModIndex` hack
  // probably also for some CRT stuff in the future
  // TODO: maybe require relatively prime?
  static_assert([]{
    std::array<std::uint32_t, sizeof...(Mods)> arr{Mods...};
    std::ranges::sort(arr);
    // could use `adjacent<2>` if C++23
    for (auto idx : std::views::iota(1_u32, arr.size()))
      if (arr[idx] == arr[idx-1])
        return false;
    return true;
  }());
  
  static constexpr std::array<std::uint32_t, sizeof...(Mods)> ModsArray{Mods...};

  template<std::uint32_t arg>
  static constexpr std::uint32_t ModIndex = std::distance(ModsArray.begin(), std::ranges::find(ModsArray, arg));
  
  std::array<std::uint32_t, sizeof...(Mods)> data;

  constexpr auto operator<=>(const MultiMint&) const = default;

  constexpr MultiMint() : data{}{}

  // hopefully compiler recognises stupidity of this and only mods once
  constexpr MultiMint(std::integral auto arg) : data{static_cast<std::uint32_t>(arg % Mods < 0 ? arg % Mods + Mods : arg % Mods)...}{}

  // constexpr MultiMint(std::integral auto ... args) requires(sizeof...(args) != 1 && sizeof...(args) == sizeof...(Mods))
  //   : data{}{}

  constexpr MultiMint(const MultiMint&) = default;
  constexpr MultiMint(MultiMint&&) = default;

  constexpr MultiMint& operator=(const MultiMint&) = default;
  constexpr MultiMint& operator=(MultiMint&&) = default;

  constexpr std::uint32_t& operator[](std::uint32_t idx){return data[idx];}
  // TODO: should this return value, not cref?
  constexpr const std::uint32_t& operator[](std::uint32_t idx) const {return data[idx];}

  // TODO: should these really be crefs, not just values?
  friend constexpr MultiMint<Mods...> operator+(const MultiMint& a, const MultiMint& b){
    return MultiMint::unsafe_create({
      (a[ModIndex<Mods>] + b[ModIndex<Mods>] < Mods
       ? a[ModIndex<Mods>] + b[ModIndex<Mods>]
       : a[ModIndex<Mods>] + b[ModIndex<Mods>] - Mods)...});
  }

  friend constexpr MultiMint<Mods...> operator-(const MultiMint& a, const MultiMint& b){
    return MultiMint::unsafe_create({
      (a[ModIndex<Mods>] >= b[ModIndex<Mods>]
       ? a[ModIndex<Mods>] - b[ModIndex<Mods>]
       : a[ModIndex<Mods>] - b[ModIndex<Mods>] + Mods)...});
  }

  friend constexpr MultiMint<Mods...> operator*(const MultiMint& a, const MultiMint& b){
    return MultiMint::unsafe_create({
      (static_cast<std::uint32_t>
       (static_cast<std::uint64_t>(a[ModIndex<Mods>]) *
        static_cast<std::uint64_t>(b[ModIndex<Mods>]) % Mods)
       )...});
  }

  friend constexpr MultiMint<Mods...> operator/(const MultiMint& a, const MultiMint& b){
    return a * MultiMint::unsafe_create({(modular_inverse(b[ModIndex<Mods>], Mods))...});
  }

  constexpr MultiMint& operator+=(const MultiMint& x){
    return *this = *this + x;
  }

  constexpr MultiMint& operator-=(const MultiMint& x){
    return *this = *this - x;
  }

  constexpr MultiMint& operator*=(const MultiMint& x){
    return *this = *this * x;
  }

  constexpr MultiMint& operator/=(const MultiMint& x){
    return *this = *this / x;
  }

  static constexpr MultiMint unsafe_create(std::array<std::uint32_t, sizeof...(Mods)> arg){
    MultiMint out;
    out.data = arg;
    return out;
  }

  constexpr explicit operator std::uint32_t() const noexcept requires(sizeof...(Mods) == 1){
    return data[0];
  }

  friend std::ostream& operator<<(std::ostream& os, const MultiMint& x){
    return os << ivl::io::Elems{x.data};
  }
  
};

} // namespace ivl::nt
