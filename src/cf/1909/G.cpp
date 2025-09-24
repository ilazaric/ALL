#include <algorithm>
#include <functional>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

#include <ivl/literals/ints>
using namespace ivl::literals::ints_exact;

#include <ivl/io/conversion>
using ivl::io::conversion::cin;

#include <ivl/logger/logger>
using namespace ivl::logger::default_logger;

#include <ivl/nt/multimint>

/*

  s = xyz
  t = xy+z
  #s = #x + #y + #z
  #t = #x + k*#y + #z, k>=2
  #t-#s = (k-1)*#y
  --> #y | #t-#s !!!

  wlog t starts and ends with s

  1) #t >= 2#s
  --> t = svs
  s = xyz
  --> xy^kz = t = xyzvxyz
  y^(k-2) = zvx
  --> s is #y-periodic (but #s not necessarily divisible by #y)
             ^ easy to test


 */

std::size_t common_prefix(std::ranges::range auto&& a, std::ranges::range auto&& b) {
  auto        ait = std::ranges::begin(a);
  auto        bit = std::ranges::begin(b);
  std::size_t out = 0;
  while (ait != std::ranges::end(a) && bit != std::ranges::end(b) && *ait == *bit)
    ++out, ++ait, ++bit;
  return out;
}

using Mint       = ivl::nt::MultiMint<1'000'000'007>;
constexpr Mint P = 13337;

int main() {
  std::ios_base::sync_with_stdio(false);

  std::uint32_t    n{cin}, m{cin};
  std::string      sdata{cin}, tdata{cin};
  std::string_view s{sdata}, t{tdata};

  {
    auto prefix = common_prefix(s, t);
    auto suffix = common_prefix(s | std::views::reverse, t | std::views::reverse);
    if (prefix + suffix <= s.size()) {
      std::cout << 0 << std::endl;
      return 0;
    }

    auto remprefix = s.size() - suffix;
    auto remsuffix = s.size() - prefix;
    s.remove_prefix(remprefix);
    s.remove_suffix(remsuffix);
    t.remove_prefix(remprefix);
    t.remove_suffix(remsuffix);
  }

  Mint thash = 0;
  for (auto c : t)
    thash = thash * P + (unsigned char)c;

  std::vector<Mint> shashes(s.size() + 1);
  shashes[0] = 0;
  for (auto i : std::views::iota(0_u32, s.size()))
    shashes[i + 1] = shashes[i] * P + (unsigned char)s[i];

  std::vector<Mint> Ppows(t.size() + 1);
  Ppows[0] = 1;
  for (auto i : std::views::iota(0_u32, t.size()))
    Ppows[i + 1] = Ppows[i] * P;

  auto shash = [&](std::uint32_t lo, std::uint32_t hi) { return shashes[hi] - shashes[lo] * Ppows[hi - lo]; };

  std::size_t sol = 0;

  LOG(s.size(), t.size());
  LOG(s, t);

  auto test_len = [&](std::uint32_t ylen) {
    LOG(ylen);
    if (ylen > s.size()) return;

    // #y-periodic test
    if (shash(0, s.size() - ylen) != shash(ylen, s.size())) return;
    LOG(ylen);

    std::uint32_t rep = (t.size() - s.size()) / ylen;
    LOG(rep);
    Mint powsum = (Ppows[rep * ylen] - 1) / (Ppows[ylen] - 1);
    Mint hash   = shash(0, s.size()) + shash(0, ylen) * Ppows[s.size()] * powsum;

    LOG((std::uint32_t)hash);
    LOG((std::uint32_t)thash);

    if (hash == thash) sol += s.size() - ylen + 1;
  };

  std::uint32_t delta = t.size() - s.size();
  for (std::uint32_t d = 1; d * d <= delta; ++d)
    if (delta % d == 0) {
      test_len(d);
      if (delta / d != d) test_len(delta / d);
    }

  std::cout << sol << std::endl;
}
