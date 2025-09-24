#include <algorithm>
#include <iomanip>
#include <numeric>
#include <ranges>
#include <span>

#include <ivl/logger/logger>
using namespace ivl::logger::default_logger;

#include <ivl/literals/ints>
using namespace ivl::literals::ints_exact;

#include <ivl/io/conversion>
using ivl::io::conversion::cin;

#include <ivl/io/stlutils>
using namespace ivl::io;

/*

  1 -> 100%

  2 -> 50% max

  3:
  {[a] b c}
  33% a
  33% {c}
  33% {b}
  3 -> (a + b + c) / 3

  4:
  {[a] b c d}
  25% a
  25% {c d} -> 12.5% max(c,d)
  25% {b d}
  25% {b c}
  (a + max(b,c)/2 + max(b,d)/2 + max(c,d)/2) / 4

  x > y > z
  x + y/2
  might as well take largest

  1/4 1/4 1/8 0

  6:
  {[a] b c d e f}
  1/6 a
  1/6 {tail-b}

  just assuming taking max is correct always wtv

  1/6 (a
  + 1/4 c + 1/4 d + 1/8 e
  + 1/4 b + 1/4 d + 1/8 e
  + 1/4 b + 1/4 c + 1/8 e
  + 1/4 b + 1/4 c + 1/8 d
  + 1/4 b + 1/4 c + 1/8 d)

  1/6 a + 1/6 b + 1/6 c + 1/6 3/4 d + 1/6 3/8 e
  1/6 a + 1/6 b + 1/6 c + 1/8 d + 1/16 e

  N=2n+3:
  {[a] b c d e ...}
  1/N a
  1/N {tail-b} -> 1/N * 1/(N-2) * sum{tail-b}
  1/N {tail-c}
  ...

  1/N * (a + 1/(N-2) * (sum{tail-b} + sum{tail-c} + ...))
  1/N * (a + 1/(N-2) * ((N-1) * sum{tail} - sum{tail}))
  1/N * (a + 1/(N-2) * ((N-2) * sum{tail}))
  1/N * (a + sum{tail})
  1/N * sum !!! (for odd N)

 */

std::int64_t gcd(std::int64_t a, std::int64_t b) {
  while (b)
    a %= b, std::swap(a, b);
  return abs(a);
}

struct Frac {
  std::int64_t num, den;
  Frac() : num(0), den(1) {}
  Frac(std::int64_t num) : num(num), den(1) {}
  Frac(std::int64_t pnum, std::int64_t pden) {
    auto g = gcd(pnum, pden);
    num    = pnum / g;
    den    = pden / g;
  }

  friend Frac operator+(Frac a, Frac b) { return {a.num * b.den + b.num * a.den, a.den * b.den}; }
  friend Frac operator-(Frac a, Frac b) { return {a.num * b.den - b.num * a.den, a.den * b.den}; }
  friend Frac operator*(Frac a, Frac b) { return {a.num * b.num, a.den * b.den}; }
  friend Frac operator/(Frac a, Frac b) { return {a.num * b.den, a.den * b.num}; }

  Frac& operator+=(Frac a) { return *this = *this + a; }
  Frac& operator*=(Frac a) { return *this = *this * a; }
  Frac& operator/=(Frac a) { return *this = *this / a; }
};

std::ostream& operator<<(std::ostream& out, Frac f) {
  if (f.num) return out << f.num << "/" << f.den;
  else return out << 0;
}

/*

  f(2, *) = {1/2, 0}

  f(2n, 0) = 1/2n
  f(2n, k+1) = k/2n * f(2n-2, k-1) + (2n-2-k)/2n * f(2n-2, k)

 */

int main() {
#define Frac double
#if 0
  { std::vector<Frac> probs;
    for (auto rep : std::views::iota(0, 5)){
      std::vector<Frac> nprobs(probs.size()+2);
      nprobs[0] = Frac{1}/Frac{(std::int64_t)nprobs.size()};
      for (auto dropped : std::views::iota(1_u32, nprobs.size()))
        for (auto curr : std::views::iota(1_u32, nprobs.size()))
          if (dropped != curr)
            nprobs[curr] += probs[curr-1-(dropped<curr)] / Frac{(std::int64_t)nprobs.size()};
      probs = std::move(nprobs);
      std::cout << Elems{probs} << std::endl;
      // auto cpy = probs;
      // for (auto& elem : cpy)
      //   elem = Frac{1, (std::int64_t)probs.size()} - elem;
      // std::cout << cpy << std::endl;
    }
  }
#endif
#undef Frac

  std::array<std::array<double, 5001>, 2501> fpre{};
  {
    // fpre[1][0] = 0.5;
    // fpre[1][1] = 0;
    for (auto row : std::views::iota(1_u32, 2501_u32)) {
      double lam   = 0.5 / row;
      fpre[row][0] = lam;
      fpre[row][1] = (2 * row - 2) * lam * fpre[row - 1][0];
      for (auto col : std::views::iota(2_u32, 2 * row))
        fpre[row][col] =
          (col - 1) * lam * fpre[row - 1][col - 2] + (2 * row - 2 - col + 1) * lam * fpre[row - 1][col - 1];
      // if (row <= 5) std::cout << Elems{std::span(fpre[row].data(), row*2)} << std::endl;
    }
  }

  // return 0;

  for (auto ti : std::views::iota(0_u32, std::uint32_t{cin})) {
    std::uint32_t                                        n{cin};
    std::vector<std::pair<std::uint32_t, std::uint32_t>> edges{cin};

    std::vector<std::vector<std::uint32_t>> outs(n);
    for (auto [x, y] : edges)
      outs[x - 1].push_back(y - 1);

    std::vector<double> probs(n, 0.0);
    probs[n - 1] = 1.0;

    auto indexed_view = [](auto&& elems, auto&& indices) {
      return indices | std::views::transform([&elems](auto&& idx) { return elems[idx]; });
    };

    for (auto idx : std::views::iota(0_u32, n - 1) | std::views::reverse) {
      if (outs[idx].empty()) {
        probs[idx] = 0.0;
        continue;
      }

      auto subview = indexed_view(probs, outs[idx]);

      if (outs[idx].size() % 2 == 1) {
        probs[idx] = std::accumulate(subview.begin(), subview.end(), 0.0) / (double)outs[idx].size();
        continue;
      }

      std::vector vp(subview.begin(), subview.end());
      std::ranges::sort(vp);
      std::ranges::reverse(vp);
      for (std::uint32_t vi = 0; vi < vp.size(); ++vi)
        vp[vi] *= fpre[vp.size() / 2][vi];
      std::ranges::sort(vp);
      probs[idx] = std::accumulate(vp.begin(), vp.end(), 0.0);
    }

    std::cout << std::setprecision(15) << probs[0] << std::endl;
    // std::cout << Elems{probs} << std::endl;
    // LOG(outs[0].size());
    // return 0;
  }
  return 0;
}
