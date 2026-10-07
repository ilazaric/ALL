#pragma once

#include <ivl/format>
#include "concepts"
#include <stdexcept>

template<gcd_ring K>
struct Ratio {
  using is_field = void;

  bool operator==(const Ratio&) const = default;

  K up;
  K down;

  void reduce() {
    auto g = gcd(up, down);
    up /= g;
    down /= g;
  }

  Ratio() : up(), down(::identity<K>()) {}
  Ratio(const K& x) : up(x), down(::identity<K>()) {}
  Ratio(const K& x, const K& y) : up(x), down(y) { reduce(); }

  static Ratio identity() { return Ratio(::identity<K>()); }

  Ratio& operator*=(const Ratio& o) {
    up *= o.up;
    down *= o.down;
    reduce();
    return *this;
  }

  Ratio& operator*=(const K& o) {
    up *= o;
    reduce();
    return *this;
  }

  friend Ratio operator*(Ratio a, const Ratio& b) {
    a *= b;
    return a;
  }

  friend Ratio operator*(Ratio a, const K& b) {
    a *= b;
    return a;
  }

  friend Ratio operator*(const K& b, Ratio a) {
    a *= b;
    return a;
  }

  Ratio& operator/=(const Ratio& o) {
    contract_assert(o.up != K{});
    up *= o.down;
    down *= o.up;
    reduce();
    return *this;
  }

  Ratio& operator/=(const K& o) {
    contract_assert(o != K{});
    down *= o;
    reduce();
    return *this;
  }

  friend Ratio operator/(Ratio a, const Ratio& b) {
    a /= b;
    return a;
  }

  friend Ratio operator/(Ratio a, const K& b) {
    a /= b;
    return a;
  }

  Ratio& operator+=(const Ratio& o) {
    up *= o.down;
    up += o.up * down;
    down *= o.down;
    reduce();
    return *this;
  }

  Ratio& operator+=(const K& o) {
    up += o * down;
    // reduce();
    return *this;
  }

  friend Ratio operator+(Ratio a, const Ratio& b) {
    a += b;
    return a;
  }

  friend Ratio operator+(Ratio a, const K& b) {
    a += b;
    return a;
  }

  friend Ratio operator+(const K& b, Ratio a) {
    a += b;
    return a;
  }

  Ratio& operator-=(const Ratio& o) {
    up *= o.down;
    up -= o.up * down;
    down *= o.down;
    reduce();
    return *this;
  }

  Ratio& operator-=(const K& o) {
    up -= o * down;
    // reduce();
    return *this;
  }

  friend Ratio operator-(Ratio a, const Ratio& b) {
    a -= b;
    return a;
  }

  friend Ratio operator-(Ratio a, const K& b) {
    a -= b;
    return a;
  }

  friend Ratio operator-(const K& b, Ratio a) {
    a -= b;
    return a;
  }
};

template<typename K>
struct ivl::fmt_raw::formatter<Ratio<K>> {
  constexpr auto parse(auto& ctx) {
    if (ctx.begin() == ctx.end() || *ctx.begin() == '}') return ctx.begin();
    throw std::runtime_error("invalid format specifier");
  }
  auto format(const Ratio<K>& r, auto& ctx) const { return ivl::fmt::format_to(ctx.out(), "{} / {}", r.up, r.down); }
};
