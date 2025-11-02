#include <ivl/io/conversion>
#include <ivl/util>
#include <cstdint>
#include <iostream>
#include <pair>
#include <utility>
#include <vector>

using ivl::io::conversion::cin;

int main() {
  for (auto _ : std::views::iota(0ULL, size_t{cin})) {
    uint32_t n{cin};
    uint32_t m{cin};
    auto     grid   = std::vector(n, std::vector(m, '0'));
    auto     answer = std::vector(n, std::vector(m, 0U));

    ivl::util::scope_exit _{[&] {
      for (auto& row : answer) {
        for (auto el : row)
          std::cout << el << " ";
        std::cout << std::endl;
      }
    }};

    auto transpose = [&] {
      auto flip = [&](auto& vec) {
        auto vec2 = std::vector(m, std::vector(n, vec[0][0]));
        for (auto i : std::views::iota(0U, n))
          for (auto j : std::views::iota(0U, m))
            vec2[j][i] = vec[i][j];
        vec = vec2;
      };
      flip(grid);
      flip(answer);
      std::swap(n, m);
    };

    for (auto i : std::views::iota(0U, n)) {
      std::string row{cin};
      for (auto j : std::views::iota(0U, m))
        grid[i][j] = row[j];
    }

    bool has_transposed = false;
    if (n < m) {
      transpose();
      has_transposed = true;
    }

    auto last_seen = std::vector(m, std::vector(m, (uint32_t)-1));
    auto rectangles = std::vector(n, std::tuple<uint32_t, uint32_t, uint32_t>{});
    for (auto i : std::views::iota(0U, n)) {
      for (auto hi : std::views::iota(0U, m))
        if (grid[i][hi] == '1')
          for (auto lo : std::views::iota(0U, hi))
            if (grid[i][lo] == '1') {
              ivl::util::scope_exit _{[&] { last_seen[lo][hi] = i; }};
              if (last_seen[lo][hi] != (uint32_t)-1) {
                // TODO
              }
            }
    }

    //                                                area      range
    auto covering = std::vector(m, std::set<std::pair<uint32_t, uint32_t>>{});
    // TODO

    if (has_transposed) transpose();
  }
}
