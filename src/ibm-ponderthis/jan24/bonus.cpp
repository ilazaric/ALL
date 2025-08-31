#include <bit>
#include <bitset>
#include <ranges>
#include <vector>

#include <ivl/io/stlutils.hpp>

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

#include <ivl/timer/timer.hpp>

constexpr std::array<std::array<int, 4>, 4> row_signs{+1, +1, -1, -1, +1, +1, +1, -1, +1, -1, +1, +1, +1, -1, +1, -1};
constexpr std::array<std::array<int, 4>, 4> col_signs{+1, +1, +1, -1, +1, +1, -1, -1, +1, -1, -1, +1, +1, +1, +1, +1};

int row_flip_sum(const std::array<int, 16>& matrix, std::uint32_t row, std::uint32_t mask) {
  mask *= 2;
  int acc = 0;
  for (auto col : std::views::iota(0, 4))
    acc += matrix[row * 4 + col] * row_signs[row][col] * ((mask & (1 << col)) ? -1 : +1);
  return acc;
}

int col_flip_sum(std::array<int, 16>& matrix, std::uint32_t col, std::uint32_t mask) {
  mask *= 2;
  int acc = 0;
  for (auto row : std::views::iota(0, 4))
    acc += matrix[row * 4 + col] * col_signs[col][row] * ((mask & (1 << row)) ? -1 : +1);
  return acc;
}

void test() {
  std::array<int, 16> left{15, 10, 8, 12, 13, 7, 1, 11, 3, 5, 2, 9, 14, 4, 6, 16};
  std::array<int, 16> right{16, 12, 9, 14, 10, 6, 2, 8, 4, 7, 1, 11, 13, 3, 5, 15};
  for (auto mask : std::views::iota(0u, 1u << 3))
    LOG(mask, row_flip_sum(left, 0, mask), row_flip_sum(right, 0, mask));
  // exit(0);
}

int main() {
  // test();

  // flat so ivl::io::Elems works
  std::vector<std::array<int, 16>> solutions;
  for (std::array<int, 16> curr; std::cin >> ivl::io::Elems{curr};)
    solutions.push_back(curr);

  for (auto& left : solutions)
    for (auto& right : solutions)
      if (left[0] < right[0]) {
        for (auto idx : std::views::iota(0, 16))
          if (left[idx] == right[idx]) goto skip;

        {
          std::uint32_t acc           = 0;
          std::uint32_t total_mask    = 0;
          auto          get_row_index = [](std::uint32_t row, std::uint32_t pos) { return row * 7 + pos; };
          auto          get_col_index = [](std::uint32_t col, std::uint32_t pos) { return 3 + 7 * pos + col; };
          for (auto idx : std::views::iota(0, 4)) {
            std::uint32_t row_mask = 0;
            std::uint32_t col_mask = 0;
            for (auto mask : std::views::iota(0u, 1u << 3)) {
              if (row_flip_sum(left, idx, mask) == row_flip_sum(right, idx, mask))
                if (std::popcount(mask) > std::popcount(row_mask)) row_mask = mask;
              if (col_flip_sum(left, idx, mask) == col_flip_sum(right, idx, mask))
                if (std::popcount(mask) > std::popcount(col_mask)) col_mask = mask;
            }
            acc += std::popcount(row_mask) + std::popcount(col_mask);
            for (auto pos : std::views::iota(0u, 3u)) {
              if (row_mask & (1 << pos)) total_mask |= 1 << get_row_index(idx, pos);
              if (col_mask & (1 << pos)) total_mask |= 1 << get_col_index(idx, pos);
            }
          }
          if (acc >= 3 * 4 * 2 / 2) {
            LOG(ivl::io::Elems{left});
            LOG(ivl::io::Elems{right});
            LOG(std::bitset<24>{total_mask});
            LOG(acc);
            std::bitset<24> clean{"101001100101101000100110"};
            auto            out = clean ^ std::bitset<24>{total_mask};
            // LOG(total_mask&7, out[0], out[1], out[2]);
            for (auto i : std::views::iota(0u, 24u))
              std::cout << (out[i] ? '-' : '+') << ",";
            std::cerr << std::endl << std::endl;
          }
        }

      skip:;
      }
}
