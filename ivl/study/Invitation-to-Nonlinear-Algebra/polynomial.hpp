#pragma once

#include <ivl/format>
#include "concepts"
#include <vector>

// TODO: SparsePoly could be useful too? for >1 variables?

template<ring K>
struct Poly {
  std::vector<K> coefs;

  bool operator==(const Poly&) const = default;

  static Poly identity() {
    Poly p;
    p.coefs.emplace_back(::identity<K>());
    return p;
  }

  K& operator[](size_t i) { return coefs[i]; }
  const K& operator[](size_t i) const { return coefs[i]; }

  bool empty() const { return coefs.empty(); }
  size_t size() const { return coefs.size(); }

  void shrink() {
    while (!coefs.empty() && coefs.back() == K{}) coefs.pop_back();
  }

  void normalize()
    requires field<K>
  {
    if (empty()) return;                  // TODO: or contract assertion fail?
    contract_assert(coefs.back() != K{}); // TODO: or shrink()?
    for (auto& el : coefs) el /= coefs.back();
  }

  // TODO: degree()
  // ....: kinda annoying for zero polynomial, degree == -inf

  Poly& operator+=(const Poly& o) {
    if (coefs.size() < o.size()) {
      for (size_t i = 0; i < coefs.size(); ++i) coefs[i] += o[i];
      for (size_t i = coefs.size(); i < o.size(); ++i) coefs.push_back(o[i]);
    } else {
      for (size_t i = 0; i < o.size(); ++i) coefs[i] += o[i];
    }
    shrink();
    return *this;
  }

  friend Poly operator+(Poly a, const Poly& b) {
    a += b;
    return a;
  }

  Poly& operator-=(const Poly& o) {
    if (coefs.size() < o.size()) {
      for (size_t i = 0; i < coefs.size(); ++i) coefs[i] -= o[i];
      for (size_t i = coefs.size(); i < o.size(); ++i) coefs.push_back(-o[i]);
    } else {
      for (size_t i = 0; i < o.size(); ++i) coefs[i] -= o[i];
    }
    shrink();
    return *this;
  }

  friend Poly operator-(Poly a, const Poly& b) {
    a -= b;
    return a;
  }

  Poly& operator*=(const K& o) {
    for (auto& el : coefs) el *= o;
    shrink();
    return *this;
  }

  Poly& operator*=(const Poly& o) {
    if (empty()) return *this;
    if (o.empty()) {
      coefs.clear();
      return *this;
    }
    if (o.size() == 1) return *this *= o[0];
    auto r = Poly{.coefs = std::vector<K>(size() + o.size() - 1, K{})};
    for (size_t i = 0; i < size(); ++i)
      for (size_t j = 0; j < o.size(); ++j) r[i + j] += coefs[i] * o[j];
    r.shrink();
    return *this = std::move(r);
  }

  friend Poly operator*(Poly a, const Poly& b) {
    a *= b;
    return a;
  }

  friend Poly operator*(Poly a, const K& b) {
    a *= b;
    return a;
  }

  friend Poly operator*(const K& b, Poly a) {
    a *= b;
    return a;
  }

  K& back() { return coefs.back(); }
  const K& back() const { return coefs.back(); }

  Poly& operator/=(const Poly& o)
    requires div_ring<K>
  {
    contract_assert(!o.empty());
    if (o.size() > size()) {
      coefs.clear();
      return *this;
    }
    for (size_t i = size() - 1; i - o.size() + 2; --i) {
      auto c = coefs[i] / o.back();
      for (size_t j = 0; j < o.size(); ++j) coefs[i - j] -= o[o.size() - j - 1] * c;
      coefs[i] = std::move(c);
    }
    coefs.erase(coefs.begin(), coefs.begin() + o.size() - 1);
    shrink();
    return *this;
  }

  Poly& operator/=(const K& o)
    requires div_ring<K>
  {
    contract_assert(o != K{});
    for (auto& el : coefs) el /= o;
    shrink();
    return *this;
  }

  friend Poly operator/(Poly a, const Poly& b)
    requires div_ring<K>
  {
    a /= b;
    return a;
  }

  friend Poly operator/(Poly a, const K& b)
    requires div_ring<K>
  {
    a /= b;
    return a;
  }
};

template<field K>
Poly<K> gcd(Poly<K> a, Poly<K> b) {
again:
  if (b.size() > a.size()) std::swap(a, b);
  if (b.empty()) {
    a.normalize();
    return a;
  }
  K c = a.back() / b.back();
  for (size_t i = 0; i < b.size(); ++i) a[a.size() - b.size() + i] -= b[i] * c;
  contract_assert(a.back() == K{});
  a.shrink();
  goto again;
}

template<typename K>
struct ivl::fmt_raw::formatter<Poly<K>> {
  std::string_view var = "x";
  ivl::fmt_raw::formatter<K> under;
  constexpr auto parse(auto& ctx) {
    auto it = ctx.begin();
    while (it != ctx.end() && *it != '}' && *it != ':') ++it;
    if (it != ctx.begin()) var = std::string_view(ctx.begin(), it);
    if (it != ctx.end() && *it == ':') ++it;
    ctx.advance_to(it);
    return under.parse(ctx);
  }
  auto format(const Poly<K>& p, auto& ctx) const {
    if (p.empty()) return std::format_to(ctx.out(), "0");
    std::string s;
    for (size_t i = 0; i < p.size(); ++i) {
      if (p[i] == K{}) continue;
      if (!s.empty()) s += " + ";
      std::string c;
      {
        ivl::fmt::basic_format_context<std::back_insert_iterator<std::string>, char> ctx(
          std::back_inserter(c), {} // ivl::fmt::make_format_args(p[i])
        );
        under.format(p[i], ctx);
      }
      if (c.contains('-') || c.contains('+')) s += "(" + c + ")";
      else s += c;
      ivl::fmt::format_to(std::back_inserter(s), " {}^{}", var, i);
    }
    if (s.empty()) s = "0";
    return ivl::fmt::format_to(ctx.out(), "{}", s);
  }
};
