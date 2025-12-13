#include <algorithm>
#include <functional>
#include <numeric>
#include <ranges>
#include <vector>

#include <ivl/io/stlutils>

#include <ivl/io/conversion>
using ivl::io::conversion::cin;

#include <ivl/logger/logger>
using namespace ivl::logger::default_logger;

#include <ivl/nt/util>

int main() {
  for (auto ti : std::views::iota(0, (int)cin)) {
    std::uint32_t             n{cin};
    std::int64_t              k{cin};
    std::vector<std::int64_t> a(n);
    std::cin >> ivl::io::Elems{a};
    for (auto& el : a)
      el -= k;

    bool has_neg  = std::ranges::any_of(a, [](std::int64_t el) { return el < 0; });
    bool has_zero = std::ranges::any_of(a, [](std::int64_t el) { return el == 0; });
    bool has_pos  = std::ranges::any_of(a, [](std::int64_t el) { return el > 0; });
    if (has_neg + has_zero + has_pos > 1) {
      std::cout << -1 << std::endl;
      continue;
    }

    if (has_zero) {
      std::cout << 0 << std::endl;
      continue;
    }

    if (has_neg)
      for (auto& el : a)
        el = -el;

    auto g = std::accumulate(a.begin(), a.end(), 0ULL, [](auto x, auto y) { return ivl::nt::gcd(x, y); });

    // LOG(n, k, a, g);

    std::cout << std::accumulate(a.begin(), a.end(), 0LL) / g - n << std::endl;
  }
}
