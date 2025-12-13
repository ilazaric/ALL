#include <algorithm>
#include <functional>
#include <numeric>
#include <ranges>
#include <vector>

#include <ivl/literals/ints>
using namespace ivl::literals::ints_exact;

#include <ivl/io/stlutils>

#include <ivl/io/conversion>
using ivl::io::conversion::cin;

#include <ivl/logger/logger>
using namespace ivl::logger::default_logger;

int main() {
  std::vector<std::vector<std::uint32_t>> valid_masks(20);
  for (auto n : std::views::iota(0_u32, valid_masks.size()))
    for (auto mask : std::views::iota(1_u32, 1_u32 << n)) {
      std::vector<std::uint32_t> divs(n + 1);
      for (auto i : std::views::iota(1_u32, n + 1))
        if (mask & (1_u32 << (i - 1)))
          for (auto j = i; j <= n; j += i)
            divs[j] ^= 1;
      auto cnt = std::ranges::count(divs, 1);
      if (cnt <= n / 5) valid_masks[n].push_back(mask);
    }

  for (auto ti : std::views::iota(0, (int)cin)) {
    std::uint32_t                                        n{cin};
    std::vector<std::pair<std::uint32_t, std::uint32_t>> edges{cin};

    if (n >= valid_masks.size()) {
      std::cout << n << std::endl;
      std::cout << ivl::io::Elems{std::views::iota(1_u32, n + 1)} << std::endl;
      continue;
    }

    for (auto mask : valid_masks[n]) {
      for (auto [x, y] : edges)
        if ((mask & (1_u32 << (x - 1))) && !(mask & (1_u32 << (y - 1)))) goto label_mask_failure;

      std::cout << std::popcount(mask) << std::endl;
      for (auto i : std::views::iota(0_u32, n))
        if (mask & (1_u32 << i)) std::cout << i + 1 << " ";
      std::cout << std::endl;
      goto label_success;

    label_mask_failure:;
    }

    std::cout << -1 << std::endl;

  label_success:;
  }
}
