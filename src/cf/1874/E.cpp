
/*

  f(n,lim)

  n -->
  0 x n-1
  1 x n-2
  ...
  n-1 x 0

  f(n,*) can be represented as a generating function
  f(n) = x^n * sum{f(k) * f(n-1-k)}

  f(0) = 1
  f(1) = x
  f(2) = 2x^3
  f(3) = x^3 * (2*2x^3 + x^2)
  = 4x6 + x5

  max power should be O(n^2)
  max power should be n+n-1+...+1 = n(n+1)/2
  ^ coef should be 2^(n-1)
  n is <= 200, seems kinda small

  y^n f(n) = y x^n sum{y^k f(k) y^(n-1-k) f(n-1-k)}
  g = sum {y^k f(k)}

  g = sum{yk fk} = 1 + sum_k!=0{y xk sum{yl fl y(k-1-l)f(n-1-l)}}
  g = 1 + sum{y xk sum{yl fl yl' fl'}}
  g = 1 + sum{y xk g[y(k-1)] * y(k-1)}
  g = 1 + sum{y x(k+1) g[yk] yk}
  g = 1 + xy sum{g[yk] yk xk}
  g(x,y) = 1 + xy g(x,xy)

  f(n+1) = g(x,y)[yn+1] = x g(x,xy) [yn]


  fn = xn * sum{fk f(n-1-k)}
  f = (f0,f1,...)
  (f * f) [n-1] * xn = fn

  g(x,y) = sum{yn fn}
  (f*f)[n] = (g*g)[yn]

  gg[yn-1] * xn = g[yn]
  gg[yn] * xn+1 = g[yn+1]
  gg[yn] xn x = g[yn+1]
  yn xn gg[yn] xy = yn+1 g[yn+1]
  xy * gg(x,xy) = g - g[y0]
  xy gg(x,xy) = g(x,y) - 1
  seems more correct
  also seems annoying

  whoops forgot choose

  fn = xn sum{fk f(n-1-k) C(n-1,k)}
  fn/n! = xn/n sum {fk/k! fk'/k'!}
  g(x,y) = sum{yn/n! fn}
  g[yn] = xn/n sum{g[yk] g[yk']}
  g[yn] = xn/n gg[yn-1]
  xn is pissing me off

  fn = x^p(n) hn
  p(n) = n + p(k) + p(n-1-k)
  p(n) = n + p(0) + p(n-1)
  p(n) = n+c + p(n-1)
  p(n) = n+c + n-1+c + ... + 1+c + c
  p(n) = n(n+1)/2 + (n+1)c
  doesnt work

  n g[yn] = xn gg[yn-1]

  (y dy(g)) [yn] = dy(g) [yn-1] = n g[yn]

  (y dyg) [yn] = xn gg[yn-1]
  (y dyg) [yn+1] = x xn gg[yn]
  (y dyg) - (y dyg)[y0] = x gg(x,xy)
  y dyg = x gg(x,xy)
  ew


  Tn = n(n+1)/2
  n >= 1: fn[xTn] = 2^(n-1)

  deg fn = Tn

  ---------

  how karatsuba?
  p*q
  p = ar+b
  q = cr+d

  pq = ac r2 + (ad+bc) r + bd
  pq = X r2 + Y r + Z

  ac = X
  bd = Z
  (a+b)(c+d) = ac+ad+bc+bd = X+Y+Z

  what if deg p != deg q

 */

#include <algorithm>
#include <bit>
#include <bitset>
#include <complex>
#include <concepts>
#include <iomanip>
#include <numbers>
#include <ranges>
#include <set>
#include <span>
#include <utility>
#include <vector>

#include <ivl/io/stlutils.hpp>
using namespace ivl::io;

#include <ivl/io/conversion.hpp>
using ivl::io::conversion::cin;

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

#include <ivl/literals/ints.hpp>
using namespace ivl::literals::ints_exact;

#include <ivl/algos/fft.hpp>
#include <ivl/nt/multimint.hpp>
#include <ivl/nt/util.hpp>

constexpr std::array<std::uint32_t, 3> Primes {
  1000112129, 1000210433, 1000308737,
  // 1000800257
};

constexpr std::uint32_t NttLen = 1 << 15;

constexpr std::array<std::uint32_t, Primes.size()> Generators = [] {
  std::array<std::uint32_t, Primes.size()> out {};
  for (auto idx : std::views::iota(0_u32, Primes.size())) {
    if (Primes[idx] % NttLen != 1)
      throw "whoops";

    for (auto candidate : std::views::iota(2_u32, Primes[idx])) {
      auto mul = ivl::nt::multiplies {Primes[idx]};
      auto r   = ivl::nt::power(candidate, (Primes[idx] - 1) / NttLen, mul);
      auto a   = ivl::nt::power(r, NttLen / 2, mul);
      if (a == 1)
        continue;
      if (ivl::nt::power(candidate, Primes[idx] - 1, mul) != 1)
        throw "whoops4";
      if (mul(a, a) != 1)
        throw "whoops3";
      out[idx] = r;
      break;
    }

    if (out[idx] == 0)
      throw "whoops2";
  }
  return out;
}();

std::array<std::uint32_t, Primes.size()> GenInverses = [] {
  std::array<std::uint32_t, Primes.size()> out;
  for (auto idx : std::views::iota(0_u32, Primes.size())) {
    auto          e = Primes[idx] - 2;
    std::uint64_t a = Generators[idx];
    auto          r = 1_u64;
    while (e) {
      if (e & 1)
        r = (a * r) % Primes[idx];
      a = (a * a) % Primes[idx];
      e >>= 1;
    }
    out[idx] = (std::uint32_t)r;
  }
  return out;
}();

constexpr std::uint32_t Mod = 1'000'000'007;

// could be MultiMint<Primes.[:]> if brevzin generalized packs get accepted
using FMint = ivl::nt::MultiMint<Primes[0], Primes[1], Primes[2]>; //, Primes[3]>;

FMint Generator  = FMint::unsafe_create(Generators);
FMint GenInverse = FMint::unsafe_create(GenInverses);

using Mint = ivl::nt::MultiMint<Mod>;

void convert_to_dft(std::span<const Mint, NttLen> in, std::span<FMint, NttLen> out) {
  SCOPE_TIMER;
  std::array<FMint, NttLen> mid;
  for (auto idx : std::views::iota(0_u32, NttLen))
    mid[idx] = in[idx][0];
  ivl::algos::fft<FMint>(mid.data(), out.data(), NttLen, Generator);
}

constexpr FMint NttLenInverse = FMint::unsafe_create({
  ivl::nt::modular_inverse(NttLen, Primes[0]), ivl::nt::modular_inverse(NttLen, Primes[1]),
  ivl::nt::modular_inverse(NttLen, Primes[2]),
  // ivl::nt::modular_inverse(NttLen, Primes[3])
});

ivl::nt::GarnerTable gt(Primes);

void convert_from_dft(std::span<const FMint, NttLen> in, std::span<Mint, NttLen> out) {
  SCOPE_TIMER;
  std::array<FMint, NttLen> mid;
  ivl::algos::fft<FMint>(in.data(), mid.data(), NttLen, GenInverse);
  for (auto& el : mid)
    el *= NttLenInverse;
  for (auto idx : std::views::iota(0_u32, NttLen)) {
    auto gv  = gt.convert_representation(mid[idx].data.data());
    out[idx] = gt.template apply<Mint>(gv);
  }
}

int main() {
  constexpr auto MAXN = 200_u32;

  std::array<std::array<Mint, MAXN + 1>, MAXN + 1> C {};
  for (auto a : std::views::iota(0_u32, MAXN + 1))
    for (auto b : std::views::iota(0_u32, MAXN + 1)) {
      if (b == 0 || b == a)
        C[a][b] = 1;
      else if (b > a)
        C[a][b] = 0;
      else
        C[a][b] = C[a - 1][b - 1] + C[a - 1][b];
    }

  std::array<std::array<FMint, MAXN + 1>, MAXN + 1> FC;
  for (auto a : std::views::iota(0_u32, MAXN + 1))
    for (auto b : std::views::iota(0_u32, MAXN + 1))
      FC[a][b] = C[a][b][0];

  std::array<std::array<Mint, NttLen>, MAXN + 1>  genfns {};
  std::array<std::array<FMint, NttLen>, MAXN + 1> dfts {};

  genfns[0][0] = 1;
  convert_to_dft(genfns[0], dfts[0]);

  for (auto n : std::views::iota(1_u32, MAXN + 1)) {
    {
      SCOPE_TIMER;
      for (auto k : std::views::iota(0_u32, n))
        for (auto idx : std::views::iota(0_u32, NttLen))
          dfts[n][idx] += dfts[k][idx] * dfts[n - 1 - k][idx]; // * FC[n-1][k];
    }

    /*
      n-1 choose k
      = (n-1)! / k! / (n-1-k)!
      gn = fn/n!
      gn n! = sum gk k! g(n-1-k) (n-1-k)! (n-1)! / k! / (n-1-k)!
      gn n = sum gk g(n-1-k)
      gn = 1/n * sum
     */

    convert_from_dft(dfts[n], genfns[n]);

    for (auto idx : std::views::iota(n, NttLen) | std::views::reverse)
      genfns[n][idx] = genfns[n][idx - n];
    for (auto idx : std::views::iota(0_u32, n))
      genfns[n][idx] = 0;

    Mint n_inv = ivl::nt::modular_inverse(n, Mod);
    for (auto& el : genfns[n])
      el *= n_inv;

    convert_to_dft(genfns[n], dfts[n]);
  }

  std::uint32_t n {cin};
  std::uint32_t lim {cin};

  Mint acc = 0;

  if (lim < NttLen)
    for (auto idx : std::views::iota(lim, NttLen))
      acc += genfns[n][idx];

  for (auto idx : std::views::iota(1_u32, n + 1))
    acc *= idx;

  std::cout << acc[0] << std::endl;
}
