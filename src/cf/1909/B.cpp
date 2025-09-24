#include <algorithm>
#include <functional>
#include <numeric>
#include <ranges>
#include <valarray>
#include <vector>

#include <ivl/io/stlutils>

#include <ivl/io/conversion>
using ivl::io::conversion::cin;

#include <ivl/logger/logger>
using namespace ivl::logger::default_logger;

int main() {
  for (auto ti : std::views::iota(0, (int)cin)) {
    std::valarray<std::uint64_t> data{cin};
    std::ranges::sort(data);
    std::valarray<std::uint64_t> lo{data[std::slice(1, data.size(), 1)]};
    std::valarray<std::uint64_t> hi{data[std::slice(0, data.size() - 1, 1)]};
    hi -= lo;
    auto tmp = std::accumulate(std::begin(hi), std::end(hi), 0ULL, std::bit_or{});
    std::cout << (2ULL << std::countr_zero(tmp)) << std::endl;
  }
}
