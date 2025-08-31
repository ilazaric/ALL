#include <algorithm>
#include <functional>
#include <map>
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

int main() {
  for (auto ti : std::views::iota(0, (int)cin)) {
    std::vector<std::uint32_t> a{cin};
    std::uint32_t              n = a.size();
    a.insert(a.begin(), 0);

    if (a.back() != n) goto label_zero;

    {
      Mint out = 1;
      for (auto i : std::views::iota(0_u32, n)) {
        switch (a[i + 1] - a[i]) {
        case 0:
          break;
        case 1:
          out *= 2 * (i - a[i]) + 1;
          break;
        case 2:
          out *= i - a[i];
          out *= i - a[i];
          break;
        default:
          goto label_zero;
        }
      }

      std::cout << out[0] << std::endl;
      goto label_done;
    }

  label_zero:
    std::cout << 0 << std::endl;

  label_done:;
  }
}
