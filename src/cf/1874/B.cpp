#include <bitset>
#include <map>
#include <queue>
#include <ranges>
#include <utility>

#include <ivl/literals/ints.hpp>
using namespace ivl::literals::ints_exact;

#include <ivl/io/conversion.hpp>
using ivl::io::conversion::cin;

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

int main() {
  std::ios_base::sync_with_stdio(false);

  for (auto ti : std::views::iota(0_u32, std::uint32_t {cin})) {
    std::uint32_t a {cin}, b {cin}, c {cin}, d {cin}, m {cin};

    {
      bool                         early_fail = false;
      std::array<std::uint32_t, 8> values;
      constexpr std::uint32_t      dead_value = -1;
      values.fill(dead_value);
      std::vector<std::uint32_t> interesting_bit_indices;
      for (std::uint32_t idx = 0; idx < 31; ++idx) {
        bool          abit  = ((a >> idx) & 1);
        bool          bbit  = ((b >> idx) & 1);
        bool          cbit  = ((c >> idx) & 1);
        bool          dbit  = ((d >> idx) & 1);
        bool          mbit  = ((m >> idx) & 1);
        std::uint32_t code  = 4 * abit + 2 * bbit + mbit;
        std::uint32_t value = 2 * cbit + dbit;
        if (values[code] == dead_value) {
          values[code] = value;
          interesting_bit_indices.push_back(idx);
        }
        if (values[code] != value)
          early_fail = true;
      }

      if (early_fail) {
        std::cout << -1 << std::endl;
        continue;
      }

      auto compress = [=](std::uint32_t arg) {
        std::uint32_t out = 0;
        for (auto idx : interesting_bit_indices)
          out = out * 2 + ((arg >> idx) & 1);
        return out;
      };

      a = compress(a);
      b = compress(b);
      c = compress(c);
      d = compress(d);
      m = compress(m);
      // a,b,c,d,m now 8-bit
    }

    auto could_reach_1 = [=](std::uint32_t x, std::uint32_t y) {
      auto alpha = x | y | m;
      auto beta  = c | d;
      return (alpha & beta) == beta;
    };

    auto could_reach_2 = [=](std::uint32_t x, std::uint32_t y) {
      std::array<std::uint32_t, 8> values;
      constexpr std::uint32_t      dead_value = -1;
      values.fill(dead_value);
      for (auto idx : std::views::iota(0_u32, 8_u32)) {
        bool          xbit  = ((x >> idx) & 1);
        bool          ybit  = ((y >> idx) & 1);
        bool          cbit  = ((c >> idx) & 1);
        bool          dbit  = ((d >> idx) & 1);
        bool          mbit  = ((m >> idx) & 1);
        std::uint32_t code  = 4 * xbit + 2 * ybit + mbit;
        std::uint32_t value = 2 * cbit + dbit;
        if (values[code] == dead_value)
          values[code] = value;
        if (values[code] != value)
          return false;
      }
      return true;
    };

    auto could_reach = [=](std::uint32_t x, std::uint32_t y) {
      return true && could_reach_1(x, y) && could_reach_2(x, y);
    };

    std::bitset<(1_u32 << 16)>                          seen;
    std::queue<std::pair<std::uint32_t, std::uint32_t>> Q1;
    std::queue<std::pair<std::uint32_t, std::uint32_t>> Q2;
    std::uint32_t                                       curr_dist    = -1;
    bool                                                found_target = false;

    auto add = [&](std::uint32_t x, std::uint32_t y) {
      if (!could_reach(x, y))
        return;
      if (seen[(x << 8) | y])
        return;
      seen[(x << 8) | y] = true;
      Q2.emplace(x, y);
    };

    add(a, b);

    // LOG(Q1.size(), Q2.size());

    while (!Q1.empty() || !Q2.empty()) {
      if (Q1.empty()) {
        ++curr_dist;
        std::swap(Q1, Q2);
      }

      auto [x, y] = Q1.front();
      Q1.pop();

      if (x == c && y == d) {
        found_target = true;
        break;
      }

      // LOG(x, y);

      if ((x & y) != x)
        add(x & y, y);
      if ((x | y) != x)
        add(x | y, y);
      add(x, x ^ y);
      add(x, y ^ m);
    }

    if (found_target)
      std::cout << curr_dist << std::endl;
    else
      std::cout << -1 << std::endl;
  }
}
