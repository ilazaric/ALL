#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <iosfwd>

template <std::size_t N>
using Underlying = std::bitset<(1uz << N)>;

template <std::size_t N>
struct Family : Underlying<N> {
  template <typename Ptr>
  struct Iterator {
    Ptr         family;
    std::size_t idx;

    Iterator(Ptr family, std::size_t idx) : family(family), idx(idx) {}

    Iterator& operator++() {
      ++idx;
      while (idx < (1uz << N) && !family->test(idx))
        ++idx;
      return *this;
    }

    Iterator operator++(int) {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    auto operator<=>(const Iterator&) const = default;

    std::size_t operator*() const { return idx; }
  };

  auto begin() { return ++Iterator {this, -1uz}; }
  auto end() { return Iterator {this, (1uz << N)}; }

  auto begin() const { return ++Iterator {this, -1uz}; }
  auto end() const { return Iterator {this, (1uz << N)}; }
};

template <std::size_t N>
bool compatible(const Family<N>& a, const Family<N>& b) {
  for (auto ai : a)
    for (auto bi : b)
      if (!b.test(ai | bi))
        return false;
  return true;
}

template <std::size_t N>
std::vector<Family<N>> generate() {
  if constexpr (N == 0) {
    return {Family<0> {}, Family<0> {1}};
  } else {
    auto                   smaller = generate<N - 1>();
    std::vector<Family<N>> out;
    // std::vector<Family<N>> out(smaller.size());
    // for (std::size_t f = 0; f < smaller.size(); ++f)
    //   for (std::size_t idx = 0; idx < smaller[f].size(); ++idx)
    //     out[f][idx] = smaller[f][idx];
    for (auto& alpha : smaller)
      for (auto& beta : smaller)
        if (compatible(alpha, beta)) {
          auto& curr = out.emplace_back();
          for (std::size_t idx = 0; idx < (1uz << (N - 1)); ++idx) {
            curr[idx]                    = alpha[idx];
            curr[idx | (1uz << (N - 1))] = beta[idx];
          }
        }
    return out;
  }
}

template <std::size_t N>
std::ostream& operator<<(std::ostream& out, const Family<N>& f) {
  out << "{ ";
  for (auto set : f) {
    if (set == 0) {
      out << "Ø ";
      continue;
    }
    for (std::size_t idx = 0; idx < N; ++idx)
      if (set & (1uz << idx))
        out << (char)('a' + idx);
    out << " ";
  }
  out << "}";
  return out;
}

// /*
//   lexicographically maximal
//   w.r.t. order:
//   empty: 0
//   singletons: 1 2 4 8 ...
//   2-element sets:
//   ...
//  */

// template<std::size_t N>
// Family<N> canonical(const Family<N>& f);

// template<std::size_t N>
// Family<N> reduce(const Family<N>& f);
