#pragma once

#include <ivl/format>
#include <vector>

namespace ivl {
// length of a permutation is not fixed,
// permutations are kinda understood to be on [0, +inf>,
// with finite carrier.
// trailing identity points are trimmable (`shrink()`).
//
// composition is opposite of usual function composition
// (p composition q) (x) == q(p(x))
struct permutation {
  std::vector<size_t> v;

  std::strong_ordering operator<=>(const permutation& o) const {
    size_t common = std::min(v.size(), o.v.size());
    for (size_t i = 0; i < common; ++i)
      if (auto sub = v[i] <=> o.v[i]; sub != 0) return sub;
    if (v.size() > common)
      for (size_t i = 0; i < v.size(); ++i)
        if (v[i] != i) return v[i] <=> i;
    if (o.v.size() > common)
      for (size_t i = 0; i < o.v.size(); ++i)
        if (o.v[i] != i) return i <=> o.v[i];
    return std::strong_ordering::equivalent;
  }

  bool operator==(const permutation& o) const {
    size_t common = std::min(v.size(), o.v.size());
    for (size_t i = 0; i < common; ++i)
      if (v[i] != o.v[i]) return false;
    if (v.size() > common)
      for (size_t i = 0; i < v.size(); ++i)
        if (v[i] != i) return false;
    if (o.v.size() > common)
      for (size_t i = 0; i < o.v.size(); ++i)
        if (o.v[i] != i) return false;
    return true;
  }

  void shrink() {
    while (!v.empty() && v.back() + 1 == v.size()) v.pop_back();
  }

  void extend(size_t n) {
    while (v.size() < n) v.push_back(v.size());
  }

  size_t operator()(size_t i) const { return i < v.size() ? v[i] : i; }

  size_t operator[](size_t i) const { return i < v.size() ? v[i] : i; }

  size_t& operator[](size_t i) {
    extend(i + 1);
    return v[i];
  }

  permutation inverse() const {
    permutation q;
    q.v.resize(v.size());
    for (size_t i = 0; i < v.size(); ++i) q.v[v[i]] = i;
    return q;
  }

  inline static permutation cycle(size_t n) {
    if (n <= 1) return {};
    permutation p;
    p.v.resize(n);
    for (size_t i = 0; i < n; ++i) p.v[i] = i + 1;
    p.v.back() = 0;
    return p;
  }

  inline static permutation cycle(size_t lo, size_t hi) {
    if (hi == lo || hi == lo + 1) return {};
    contract_assert(hi > lo);
    permutation p;
    p.v.resize(hi);
    for (size_t i = 0; i < lo; ++i) p.v[i] = i;
    for (size_t i = lo; i < hi - 1; ++i) p.v[i] = i + 1;
    p.v.back() = lo;
    return p;
  }

  permutation& operator*=(const permutation& o) {
    extend(o.v.size());
    for (size_t i = 0; i < o.v.size(); ++i) v[i] = o.v[v[i]];
    return *this;
  }

  friend permutation operator*(permutation a, const permutation& b) {
    a *= b;
    return a;
  }

  void compose_into(const auto&... bs) { ((*this *= bs), ...); }

  friend permutation compose(permutation a, const auto&... bs) {
    ((a *= bs), ...);
    return a;
  }

  permutation power(size_t n) const {
    permutation ret;
    permutation tmp;
    while (true) {
      if (n & 1) ret *= *this;
      n >>= 1;
      if (n == 0) return ret;
      tmp.compose_into(ret, ret);
      std::swap(tmp, ret);
      tmp.v.clear();
    }
  }

  inline static permutation rnd(size_t n) {
    permutation p;
    p.v.resize(n);
    for (size_t i = 0; i < n; ++i) p.v[i] = i;
    for (size_t i = 0; i < n; ++i) std::swap(p.v[i], p.v[i + rand() % (n - i)]);
    return p;
  }

  auto begin() const { return v.begin(); }
  auto end() const { return v.end(); }
};
} // namespace ivl

template<>
struct ivl::fmt_raw::formatter<ivl::permutation> {
  ivl::fmt::formatter<std::vector<size_t>> underlying;
  constexpr auto parse(auto& ctx) { return underlying.parse(ctx); }
  auto format(const ivl::permutation& p, auto& ctx) const { return underlying.format(p.v, ctx); }
};
