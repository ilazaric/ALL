

/*

  f(0) = 1 + f(1)
  f(n) = 0

  f(k) = 1 + (f(k-1) * l(k-1 -> k) + f(k+1) * l(k -> k+1)) / (l(k-1 -> k) + l(k -> k+1))

  f(1) = 1 + f(0) * x + f(2) * (1-x)
  f(1) = 1 + (1 + f(1)) * x + f(2) * (1-x)
  (1-x) f(1) = 1 + x + f(2) (1-x)
  f(1) = (1+x) / (1-x) + f(2)

  f(2) = 1 + y f(1) + (1-y) f(3)
  f(2) = 1 + y ((1+x) / (1-x) + f(2)) + (1-y) f(3)
  (1-y) f(2) = 1 + y(1+x)/(1-x) + (1-y) f(3)
  f(2) = f(3) + ...


  Ai = l(i -> i+1)
  Ei = f(i)

  E0 = 1 + E1
  E1 = 1 + (E0 * A0 + E2 * A1) / (A0 + A1)
  E1 = 1 + (A0 + E1A0 + E2A1) / (A0+A1)
  E1A1/(A0+A1) = 1 + A0 + E2A1 / (A0+A1)
  A1 = (1+A0)(A0+A1)/A1 + E2


  a = T + b
  b = 1 + a(1-p) + cp
  b = 1 + (T+b)(1-p) + cp
  pb = 1+T(1-p) + cp
  b = (1+T(1-p))/p + c

  (T, p) --> (1+T(1-p))/p

  (1+T-Tp)/p =
  (1+T)/p - T

  p = r/(l+r)

  (1+T)(l+r)/r - T
  (1+T)l/r + (1+T) - T
  (1+T)l/r + 1


  0 -a-> 1 -b-> 2

  f(2) = 0
  f(1) = 1 + (1-x)f(0) + xf(2)
  f(1) = 1 + (1-x)f(0)
  f(0) = 1 + f(1)
  f(1) = 1 + (1-x)(1 + f(1))
  xf(1) = 2-x
  f(1) = (2-x)/x
  f(0) = 1 + f(1)
  f(0) = 2/x


  0 --a-- 1 --b-- 2 --c-- 3

  f0 = 1 + f1
  f1 = 1 + (af0 + bf2) / (a+b)
  f2 = 1 + (bf1 + cf3) / (b+c)
  f3 = 0

  f2 = 1 + bf1/(b+c)
  f1 = 1 + (af0 + bf2) / (a+b)
  f1 = 1 + (af0 + b + bbf1/(b+c)) / (a+b)
  f1 (1 - bb/(b+c)(a+b)) = 1 + (af0 + b) / (a+b)


  f1 = 1 + (af0 + bf2) / (a+b)
  f1 (a+b) = a+b + af0 + bf2
  (f1-f0) a = a+b + (f2-f1) b

  di = fi+1 - fi

  d0 = 1
  d0 a = a+b + d1 b
  d1 b = b+c + d2 c

  a = a+b + d1 b
  -1 = d1

  .........

  pi = a0 + ... + ai
  e = sum {2pi/ai - 1}
  minimize e

  f(n, m) = min{f(n-1, m-x) + 2m/x - 1}

  is this a quadrangle or smth like that
  :(

  f(n, m) >= f(n, m+1)

  f(n,m) = min{f(n-1,x) + 2m/(m-x)-1}
  c(a,b) = 2b/(b-a)+1
  f(n,m) = min{f(n-1,x) + c(x,m)}

  a<b<c<d
  ? c(a,d) + c(b,c) >= c(a,c) + c(b,d)
  ? d/(d-a) + c/(c-b) >= c/(c-a) + d/(d-b)

  d(c-b)(c-a)(d-b) +
  c(d-a)(c-a)(d-b) >=
  c(d-a)(c-b)(d-b) +
  d(d-a)(c-b)(c-a)

  1:
  d(c-b)(c-a)(d-b) =
  d(cc-ac-bc+ab)(d-b) =
  d(ccd-acd-bcd+abd - bcc+abc+bbc-abb) =
  ccdd-acdd-bcdd+abdd-bccd+abcd+bbcd-abbd

  2:
  c(d-a)(c-a)(d-b) =
  c(cd-ac-ad+aa)(d-b) =
  c(cdd-acd-add+aad - bcd+abc+abd-aab) =
  ccdd-accd-acdd+aacd-bccd+abcc+abcd-aabc

  ?
  d(c-b)(c-a)(d-b) -
  d(d-a)(c-b)(c-a) >=
  c(d-a)(c-b)(d-b) -
  c(d-a)(c-a)(d-b)

  ?
  d(c-b)(c-a)(d-b-d+a) >=
  c(d-a)(d-b)(c-b-c+a)

  ?
  d(c-b)(c-a)(a-b) >=
  c(d-a)(d-b)(a-b)

  ?
  d(c-b)(c-a) <=
  c(d-a)(d-b)

  ?
  ccd-acd-bcd+abd <=
  cdd-acd-bcd+abc

  ?
  ccd+abd <=
  cdd+abc

  cdd+abc-ccd-abd >= 0 ?
  (cd-ab)(d-c) >= 0 ?
  obvious

  seems like quadrangle works
  how does quadrangle work?
  something about tracking optimum

  let t(n,m) = argmin_x{...}

  t(n,m+1) >= t(n,m)
  t(n+1,m) >=? t(n,m)

  gonna pretend im not aware of possible multiple optimums

  suppose we computed {f,t}(n,*)
  how do we compute {f,t}(n+1,*) ?

  take mid m, compute t, left dont have to look above,
  right dont have to look below

  t -> f

 */

#include <array>
#include <iomanip>
#include <iostream>
#include <queue>
#include <ranges>

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

#include <ivl/io/conversion.hpp>
using ivl::io::conversion::cin;

#include <ivl/literals/ints.hpp>
using namespace ivl::literals::ints_exact;

constexpr auto                                     MAXN = 3000_u32;
constexpr auto                                     MAXM = 3000_u32;
constexpr auto                                     INF  = 1e100;
std::array<std::array<double, MAXM + 1>, MAXN + 1> F;
std::array<std::array<double, MAXM + 1>, MAXN + 1> T;

int main() {
  {
    auto C = [](std::uint32_t a, std::uint32_t b) {
      if (a >= b)
        return INF;
      return 2.0 * (double)b / (double)(b - a) - 1.0;
    };

    F[0].fill(0.0);
    for (auto n : std::views::iota(1_u32, MAXN + 1)) {
      F[n].fill(INF);
      std::queue<std::array<std::uint32_t, 4>> Q;
      Q.push({n, MAXM, 0_u32, MAXM});

      while (!Q.empty()) {
        auto [mlo, mhi, tlo, thi] = Q.front();
        Q.pop();
        auto m = (mlo + mhi) / 2;
        auto t = 0_u32;
        auto e = INF;
        for (auto ct : std::views::iota(tlo, thi + 1)) {
          auto ce = F[n - 1][ct] + C(ct, m);
          if (ce < e)
            e = ce, t = ct;
        }

        F[n][m] = e;
        T[n][m] = t;
        if (m != mlo)
          Q.push({mlo, m - 1, tlo, t});
        if (m != mhi)
          Q.push({m + 1, mhi, t, thi});
      }
    }
  }

  std::uint32_t n {cin}, m {cin};
  std::cout << std::setprecision(15) << F[n][m] << std::endl;

  // while (n){
  //   LOG(n, m, T[n][m], m-T[n][m]);
  //   m = T[n][m];
  //   --n;
  // }
}
