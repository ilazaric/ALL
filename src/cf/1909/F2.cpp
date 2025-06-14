#include <algorithm>
#include <functional>
#include <numeric>
#include <ranges>
#include <vector>

#include <ivl/literals/ints.hpp>
using namespace ivl::literals::ints_exact;

#include <ivl/io/stlutils.hpp>

#include <ivl/io/conversion.hpp>
using ivl::io::conversion::cin;

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

#include <ivl/nt/multimint.hpp>

constexpr std::uint32_t Mod = 998'244'353;
using Mint                  = ivl::nt::MultiMint<Mod>;

auto factorials_storage = [] {
  std::array<Mint, 200'005> out {};
  out[0] = 1;
  for (auto i : std::views::iota(1_u32, out.size()))
    out[i] = out[i - 1] * i;
  return out;
}();

Mint factorials(std::int32_t n) {
  if (n < 0)
    return 0;
  return factorials_storage[n];
}

Mint choose(std::int32_t a, std::int32_t b) {
  if (a < 0)
    return 0;
  if (b < 0)
    return 0;
  if (b > a)
    return 0;
  // LOG(a, b);
  return factorials(a) / factorials(b) / factorials(a - b);
}

Mint magic(std::int32_t full, std::int32_t banned, std::int32_t count) {
  // LOG(full, banned, count);
  if (count == 0)
    return 1;

  if (full < banned)
    return 0;

  if (count > full)
    return 0;

  Mint out = 0;
  // for (auto a : std::views::iota(0, count+1))
  //   for (auto b : std::views::iota(0, count-a+1))
  //     for (auto c : std::views::iota(count-a-b, count-a-b+1)){
  //       // LOG(a,b,c);
  //       out +=
  //         (
  //          choose(banned, a) * choose(full-banned, a) *
  //          choose(banned, b) * choose(full-banned, b) *
  //          choose(full-banned-a, c) * choose(full-banned-b, c) *
  //          factorials(a) * factorials(b) * factorials(c)
  //          );
  //     }

  /*
    banned! (full-banned)! / (full-count)!
    sum of
    (full-a choose banned) (banned choose a) (full-banned choose b) =
   */

  for (auto a : std::views::iota(0, count + 1))
    for (auto b : std::views::iota(count - a, count - a + 1))
      out += choose(full - a, banned) * choose(banned, a) * choose(full - banned, b);
  out *= factorials(banned) * factorials(full - banned) / factorials(full - count);

  // for (auto a : std::views::iota(0, count+1))
  //   for (auto b : std::views::iota(count-a, count-a+1))
  //     // LOG(a,b,c);
  //     out +=
  //       (
  //        choose(banned, a) * choose(full-banned, a) * factorials(a) *
  //        choose(full-a, b) * choose(full-banned, b) * factorials(b)
  //        );

  /*
    sum[a+b == count] of
    (banned choose a) (full-banned choose a) a!
    (full-a choose b) (full-banned choose b) b! =
    sum of
    banned! / a! / (banned-a)! (full-banned)! / a! / (full-banned-a)! a!
    (full-a)! / b! / (full-a-b)! (full-banned)! / b! / (full-banned-b)! b! =
    sum of
    banned! / (banned-a)! (full-banned)! / a! / (full-banned-a)!
    (full-a)! / (full-count)! (full-banned)! / b! / (full-banned-b)! =
    banned! (full-banned)!^2 / (full-count)!
    sum of
    1 / (banned-a)! / a! / (full-banned-a)!
    (full-a)! / b! / (full-banned-b)! =
    banned! (full-banned)!^2 / (full-count)!
    sum of
    (full-a)! / (banned-a)! / a! / (full-banned-a)!
    / b! / (full-banned-b)! =
    banned!^2 (full-banned)!^2 / (full-count)!
    sum of
    (full-a choose banned) / (banned-a)! / a!
    / b! / (full-banned-b)! =
    banned! (full-banned)! / (full-count)!
    sum of
    (banned choose a)
    (full-banned choose b)
    (full-a choose banned) =
    banned! (full-banned)! / (full-count)!
    sum of
    (full-a choose banned) (banned choose a) (full-banned choose b) =

    sum[a+b+c == count] of
    (banned choose a) (full-banned choose a) a!
    (banned choose b) (full-banned choose b) b!
    (full-banned-a choose c) (full-banned-b choose c) c!
    ==
    sum[a+b == count] of
    (banned choose a) (full-banned choose a) a!
    (full-a choose b) (full-banned choose b) b!

    [a given]
    [A=count-a]
    sum[b+c == A] of
    (banned choose b) (full-banned choose b) b!
    (full-banned-a choose c) (full-banned-b choose c) c!
    ==
    (full-a choose A) (full-banned choose A) A!

    sum[b+c == A] of
    (banned choose b)
    (full-banned-a choose c)
    ==
    (full-a choose A)

    [a given]
    sum[b+c == count-a] of
    (banned choose b)
    (full-banned-a choose c) (full-banned-b choose c) c!
    ==
    [b == count-a]
    (full-a choose b)

    should be able to prove ^

    [A = count-a]
    sum[b+c == A] of
    (banned choose b)
    (full-banned-a choose c) (full-banned-b choose c) c!
    ==?
    (full-a choose A)
    ==
    sum[b=0..=A] (banned choose b)(full-a-banned choose A-b)

    (a+b choose c) = sum (a choose t)(b choose c-t)

    wow all this time this was fast enough, thought i needed closed form
    whoops

   */

  return out;

  /*

    left=full-banned

    sum[a,b,c;a+b+c==count]
    (banned choose a) (left choose a)
    (banned choose b) (left choose b)
    (left-a choose c) (left-b choose c) =
    sum[]
    banned! / a! / (banned-a)!
    left! / a! / (left-a)!
    banned! / b! / (banned-b)!
    left! / b! / (left-b)!
    (left-a)! / c! / (left-a-c)!
    (left-b)! / c! / (left-b-c)! =
    sum[]
    banned! / a! / (banned-a)!
    left! / a!
    banned! / b! / (banned-b)!
    left! / b!
    1 / c! / (left-a-c)!
    1 / c! / (left-b-c)! =
    sum[]
    (banned choose a)
    (banned choose b)
    (left choose a,c)
    (left choose b,c) =
    sum[a]
    (banned choose a)
    sum[b,c; b+c==count-a]
    (banned choose b)
    (left choose a,c)
    (left choose b,c)

    (x choose y,z) =
    (x choose y+z) (y+z choose y)

    sum[a]
    (banned choose a)
    sum[b,c; b+c==count-a]
    (banned choose b)
    (left choose a+c)
    (a+c choose c)
    (left choose b+c)
    (b+c choose c) =
    sum[a]
    (banned choose a)
    (left choose count-a)
    sum[b,c; b+c==count-a]
    (banned choose b)
    (left choose a,c)
    (count-a choose c) =

    k>0: (x+1 choose k) = (x choose k) + (x choose k-1)
    (x+t choose k) should be interpretable as a linear combination of (x choose i), 0<=i<=k
    similar to (x+t)^k

    what about k -> (a choose k) * (b choose k) ?
    is that integrable ?

    to know what is integrable i probably need to know how differentiation works

    Dt F (t) = F(t+1) - F(t)
    It F (t) = sum[i=0..<t] F(i)
    Dt It F == F
    It Dt F (t) == F(t) - F(0)

    F(t) = (t choose k)
    Dt F (t) = (t+1 choose k) - (t choose k)
    = (t choose k-1) (if k > 0)
    = 0 (if k == 0)

    F(t) = (k choose t)
    Dt F (t) = (k choose t+1) - (k choose t)
    = k! / (t+1)! / (k-t-1)! - (k choose t)
    = k! / t! / (t+1) / (k-t)! * (k-t) - (k choose t)
    = (k choose t) * (k-t) / (t+1) - (k choose t)
    = (k choose t) * (k-2t-1) / (t+1)
    feels kinda ugly
    (k choose t+1) - (k choose t)
    = (k-1 choose t+1) + (k-1 choose t) - (k-1 choose t) - (k-1 choose t-1)
    = (k-1 choose t+1) - (k-1 choose t-1)
    k! / (t+1)! / (k-t-1)! - k! / t! / (k-t)!
    = k! / (t+1)! / (k-t)! * (k-t) - k! / (t+1)! * (t+1) / (k-t)!
    = 1 / (t+1)! / (k-t)! [k! * (k-t) - k! * (t+1)]
    = k! / (t+1)! / (k-t)! * (k-2t-1)
    = (k+1 choose t+1) * (k-2t-1) / (k+1)
    a bit less ugly i guess, no t in denominator
    k-2t-1 tho

    It (n choose t) (n+1) == 2^n

    a polynomial p(x) is a linear combination of (x choose i), i>=0
    Dt p translates coefficients left, drops i=0
    It p translates coefficients right, 0th is 0

    It (a choose t)(b choose t) (?) == (a+b choose a)

    It (a+t choose t) (t) == (a+t choose t-1)
    (a+t choose t) == (a+t choose a)


    banned
    full >= banned
    count

    F(full, banned, count) =
    sum[a+b+c == count] of
    (banned choose a) (full-banned choose a)
    (banned choose b) (full-banned choose b)
    (full-banned-a choose c) (full-banned-b choose c) =
    sum[a+b+c == count] of
    (banned choose a) (full-banned choose a,c)
    (banned choose b) (full-banned choose b,c)

    F(full, banned, count+1) - F(full, banned, count) =
    [1, c==0]
    sum[a+b == count+1]
    (banned choose a) (full-banned choose a)
    (banned choose b) (full-banned choose b)
    [1, 1<=c==c.2+1, a+b+c.2 == count]
    + sum[a+b+c == count]
    (banned choose a) (full-banned choose a,c+1)
    (banned choose b) (full-banned choose b,c+1)
    [2]
    - sum[a+b+c == count]
    (banned choose a) (full-banned choose a,c)
    (banned choose b) (full-banned choose b,c) =

    looking at wrong expression all this time
    xD

    F(full, banned, count) =
    sum[a+b+c == count] of
    (banned choose a) (full-banned choose a) a!
    (banned choose b) (full-banned choose b) b!
    (full-banned-a choose c) (full-banned-c choose c) c! =
    banned! / a! / (banned-a)! (full-banned)! / a! / (full-banned-a)! a!
    banned! / b! / (banned-b)! (full-banned)! / b! / (full-banned-b)! b!
    (full-banned-a)! / c! / (full-banned-a-c)!
    (full-banned-b)! / c! / (full-banned-b-c)!
    c! =
    banned! / (banned-a)! (full-banned)! / a!
    banned! / (banned-b)! (full-banned)! / b!
    / c! / (full-banned-a-c)!
    / (full-banned-b-c)! =
    banned! / (banned-a)! (full-banned)! / a!
    banned! / (banned-b)! (full-banned)! / b!
    / (count-a-b)! / (full-banned-count+b)!
    / (full-banned-count+a)! =
    banned! banned! (full-banned)! (full-banned)! /
    (a! (banned-a)! b! (banned-b)! (count-a-b)! (full-banned-count+a)! (full-banned-count+b)) =
    (banned choose a)
    (banned choose b)
    (full-banned full-banned | count-a-b full-banned-count+a full-banned-count+b count)
    count! =
    (banned choose a)
    (banned choose b)
    (full-banned choose count-a)
    (full-banned choose count-b)
    (count-a)!
    (count-b)!
    / (count-a-b)!

    [c==0]:
    sum[a+b == count] of
    banned! / (banned-a)! (full-banned)! / a!
    banned! / (banned-b)! (full-banned)! / b!
    / (full-banned-a)!
    / (full-banned-b)! =
    linear convolution square of t -> (banned choose t) (full-banned choose t) t!
    similar to (full-t choose t,banned-t)

    F(full, 0, count) =
    (full choose count)^2 count!

    F(full, banned, 1) =
    full^2 - banned^2

    F(full, banned, count) =
    (full choose count)^2 * count! # counts as if banned=0
    - sum[banned_count=1..=count] of
    (banned choose banned_count)^2 * banned_count!
    F(full-banned_count, banned-banned_count, count-banned_count)

    F(full, banned, count) =
    (full choose count)^2 count! if banned == 0
    F(full-1, banned, count) +
    F(full-1, banned, count-1) +
    F(full-1, banned-1, count-2) * banned^2 +
    F(full-2, banned, count-2) * (full-banned-1)^2

    F(full, banned, 2) =
    (banned choose 2) (full-banned choose 2) * 2! +
    banned * (full-banned) * (full-1) * (full-banned-1) +
    (full choose 2) * (full-banned choose 2) * 2!

    F(full, banned, count) =
    sum[a+b == count] of
    (banned choose a) (full-banned choose a) a!
    (full-a choose b) (full-banned choose b) b!

   */
}

int main() {
  // for (auto c : std::views::iota(0, 20))
  //   LOG(c, (std::uint32_t)magic(5, 2, c));
  // exit(0);

  for (auto ti : std::views::iota(0, (int)cin)) {
    std::vector<std::int32_t> a {cin};
    // if (ti != 3) continue;
    std::uint32_t n = a.size();
    a.insert(a.begin(), 0);

    if (a.back() == -1)
      a.back() = n;

    if (a.back() != n) {
      std::cout << 0 << std::endl;
      continue;
    }

    Mint          out      = 1;
    std::uint32_t last_pos = 0;

    for (auto pos : std::views::iota(1_u32, n + 1))
      if (a[pos] != -1) {
        if (a[pos] > pos) {
          out = 0;
          break;
        }

        std::uint32_t full   = pos - a[last_pos];
        std::uint32_t banned = last_pos - a[last_pos];
        std::uint32_t count  = a[pos] - a[last_pos];
        out *= magic(full, banned, count);
        last_pos = pos;
      }

    std::cout << out[0] << std::endl;
    // exit(0);
  }
}
