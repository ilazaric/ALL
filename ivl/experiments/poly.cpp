#include <ivl/logger>
#include <ivl/util>
#include <cassert>
#include <concepts>
#include <gmpxx.h>
#include <iostream>
#include <type_traits>
#include <vector>

// IVL add_compiler_flags_tail("-lgmpxx -lgmp")

template <typename T>
struct polynomial {
  std::vector<T> data;

  polynomial(auto&&... args)
    requires((sizeof...(args) > 1) || ... || !std::same_as<std::decay_t<decltype(args)>, polynomial>)
      : data{FWD(args)...} {}

  polynomial() = default;
  polynomial(const polynomial&) = default;
  polynomial(polynomial&&) = default;
  polynomial& operator=(const polynomial&) = default;
  polynomial& operator=(polynomial&&) = default;
  ~polynomial() = default;

  bool empty() const { return data.empty(); }

  size_t size() const { return data.size(); }

  T& operator[](size_t idx) { return data[idx]; }
  const T& operator[](size_t idx) const { return data[idx]; }

  void trim() {
    while (!data.empty() && data.back() == 0) data.pop_back();
  }

  polynomial& operator+=(const polynomial& o) {
    while (size() < o.size()) data.emplace_back();
    for (size_t i = 0; i < o.size(); ++i) data[i] += o[i];
    trim();
    return *this;
  }

  friend polynomial operator+(polynomial a, const polynomial& b) {
    a += b;
    return a;
  }

  polynomial& operator-=(const polynomial& o) {
    while (size() < o.size()) data.emplace_back();
    for (size_t i = 0; i < o.size(); ++i) data[i] -= o[i];
    trim();
    return *this;
  }

  friend polynomial operator-(polynomial a, const polynomial& b) {
    a -= b;
    return a;
  }

  friend polynomial operator*(const polynomial& a, const polynomial& b) {
    if (a.empty() || b.empty()) return {};
    polynomial res;
    res.data.resize(a.size() + b.size() - 1);
    for (size_t i = 0; i < a.size(); ++i)
      for (size_t j = 0; j < b.size(); ++j) res[i + j] += a[i] * b[j];
    res.trim(); // only really needed with rings with zero divisors
    return res;
  }

  polynomial& operator*=(const polynomial& o) { return *this = *this * o; }

  // only use this if field
  polynomial& operator/=(const T& coef) {
    for (auto& el : data) el /= coef;
    trim();
    return *this;
  }

  friend polynomial operator/(polynomial a, const T& coef) {
    a /= coef;
    return a;
  }

  friend polynomial operator/(polynomial a, const polynomial& b) {
    // LOG(a, b);
    if (a.size() < b.size()) return {};
    polynomial res;
    res.data.resize(a.size() - b.size() + 1);
    for (size_t i = a.size() - 1; i + 1 >= b.size(); --i) {
      if (a.size() <= i) continue;
      auto coef = a[i] / b.data.back();
      // LOG(i, coef);
      res[i - b.size() + 1] = coef;
      for (size_t j = 0; j < b.size(); ++j) {
        a[i - b.size() + 1 + j] -= coef * b[j];
      }
      a.trim();
      // LOG(a);
      assert(a.size() <= i);
    }
    assert(a.empty());
    return res;
  }

  polynomial& operator/=(const polynomial& o) { return *this = *this / o; }
};

template <typename T>
polynomial<T> gcd(polynomial<T> a, polynomial<T> b) {
  if (a.empty()) return b;
  while (!b.empty()) {
    b /= auto(b.data.back());
    polynomial<T> c;
    for (size_t i = b.size() - 1; i < a.size(); ++i) c.data.push_back(a[i]);
    a -= b * c;
    std::swap(a, b);
  }
  return a;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const polynomial<T>& poly) {
  out << "[ ";
  for (auto&& el : poly.data) out << el << " ";
  return out << "]";
}

template <typename T>
struct rational {
  polynomial<T> numerator;
  polynomial<T> denominator;

  rational(const polynomial<T>& numerator, const polynomial<T>& denominator = {1})
      : numerator(numerator), denominator(denominator) {
    trim();
  }

  void trim() {
    auto g = gcd(numerator, denominator);
    numerator /= g;
    denominator /= g;
    auto coef = denominator.data.back();
    numerator /= coef;
    denominator /= coef;
  }
};

template <typename T>
std::ostream& operator<<(std::ostream& out, const rational<T>& r) {
  return out << r.numerator << " / " << r.denominator;
}

int main() {
  // mpq_class b("1000");
  // mpq_class c("999");
  // mpq_class r = 1;
  // for (int i = 0; i < 100; ++i) r *= b / c;
  // std::cout << r << std::endl;

  using P = polynomial<mpq_class>;
  P a{1, 1};
  LOG(a);
  LOG(a * a);
  LOG(gcd(a, a * a));
  LOG((a * a) / gcd(a, a * a));

  LOG(rational(a, a * a));
}
