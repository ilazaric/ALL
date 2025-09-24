#include <algorithm>
#include <ranges>
#include <vector>

#include <ivl/io/stlutils>

#include <ivl/io/conversion>
using ivl::io::conversion::cin;

#include <ivl/logger/logger>
using namespace ivl::logger::default_logger;

int main() {
  for (auto ti : std::views::iota(0, (int)cin)) {
    std::vector<std::pair<int, int>> pts{cin};
    int                              count = 0;
    count += std::ranges::min(pts, std::ranges::less{}, &std::pair<int, int>::first).first < 0;
    count += std::ranges::max(pts, std::ranges::less{}, &std::pair<int, int>::first).first > 0;
    count += std::ranges::min(pts, std::ranges::less{}, &std::pair<int, int>::second).second < 0;
    count += std::ranges::max(pts, std::ranges::less{}, &std::pair<int, int>::second).second > 0;
    std::cout << (count == 4 ? "NO" : "YES") << std::endl;
  }
}
