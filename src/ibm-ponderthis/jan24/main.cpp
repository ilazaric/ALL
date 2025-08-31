#include <bit>
#include <bitset>
#include <ranges>
#include <vector>

#include <ivl/io/stlutils.hpp>

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

#include <ivl/timer/timer.hpp>

namespace v1 {
  std::vector<int>                  stack;
  std::array<std::array<int, 4>, 4> matrix;
  std::array<int, 4>                row_target{5, 10, 9, 0};
  std::array<int, 4>                col_target{17, 8, 11, 48};
  std::array<std::array<int, 4>, 4> row_signs{+1, +1, -1, -1, +1, +1, +1, -1, +1, -1, +1, +1, +1, -1, +1, -1};
  std::array<std::array<int, 4>, 4> col_signs{+1, +1, +1, -1, +1, +1, -1, -1, +1, -1, -1, +1, +1, +1, +1, +1};

  bool test_validity() {
    for (auto row : std::views::iota(0, 4)) {
      int acc = 0;
      for (auto col : std::views::iota(0, 4))
        acc += row_signs[row][col] * matrix[row][col];
      if (acc != row_target[row]) return false;
    }

    for (auto col : std::views::iota(0, 4)) {
      int acc = 0;
      for (auto row : std::views::iota(0, 4))
        acc += col_signs[col][row] * matrix[row][col];
      if (acc != col_target[col]) return false;
    }

    return true;
  }

  void recursive_search() {
    if (stack.empty()) {
      if (test_validity()) {
        for (auto& row : matrix) {
          for (auto cell : row)
            std::cout << cell << " ";
          std::cout << std::endl;
        }
        std::cout << std::endl;
      }
      return;
    }

    auto mpos = 16 - stack.size();
    auto row  = mpos % 4;
    auto col  = mpos / 4;

    if (row != 0 && col == 0) {
      int acc = 0;
      for (auto idx : std::views::iota(0, 4))
        acc += row_signs[row - 1][idx] * matrix[row - 1][idx];
      if (acc != row_target[row - 1]) return;
    }

    for (auto idx : std::views::iota(0u, stack.size())) {
      auto curr = stack[idx];
      std::swap(stack[idx], stack.back());
      stack.pop_back();
      matrix[row][col] = curr;
      recursive_search();
      stack.push_back(curr);
      std::swap(stack[idx], stack.back());
    }
  }
} // namespace v1

namespace v2 {
  std::uint32_t mask = 0xFFFF;
  std::uint32_t row  = 0;
  std::uint32_t col  = 0;

  void next() {
    if (col == 3) col = row++;
    else if (row == 3) row = ++col;
    else ++(row <= col ? col : row);
  }

  void prev() {
    if (col + 1 == row) col = 3, --row;
    else if (row == col) row = 3, --col;
    else --(row < col ? col : row);
  }

  // diff <(./main 2>&1 | cut -d ':' -f 3) <(./main 2>&1 | tac | cut -d ':' -f 3)
  void test_next_prev() {
    LOG(row, col);
    while (row != 3 || col != 3) {
      next();
      LOG(row, col);
    }
    while (row != 0 || col != 0) {
      prev();
      LOG(row, col);
    }
  }

  int last_row_cell() {
    int acc = 0;
    for (auto idx : std::views::iota(0, 3))
      acc += v1::row_signs[row][idx] * v1::matrix[row][idx];
    return v1::row_signs[row][3] * (v1::row_target[row] - acc);
  }

  int last_col_cell() {
    int acc = 0;
    for (auto idx : std::views::iota(0, 3))
      acc += v1::col_signs[col][idx] * v1::matrix[idx][col];
    return v1::col_signs[col][3] * (v1::col_target[col] - acc);
  }

  void recursive_search() {
    SCOPE_TIMER;
    // LOG(row, col);

    if (row == 3 && col == 3) {
      // TODO
      int a = last_row_cell();
      int b = last_col_cell();
      if (a != b) return;
      if (a <= 0 || a > 16) return;
      if ((mask & (1 << (a - 1))) == 0) return;
      v1::matrix[row][col] = a;
      for (auto& row : v1::matrix) {
        for (auto cell : row)
          std::cout << cell << " ";
        std::cout << std::endl;
      }
      std::cout << std::endl;
      return;
    }

    if (col == 3) {
      int a = last_row_cell();
      // LOG(row, col, a);
      // if (row == 0)
      //   LOG(row, col, a, ivl::io::Elems{v1::matrix[row]});
      if (a <= 0 || a > 16) return;
      // LOG(row, col, a);
      if ((mask & (1 << (a - 1))) == 0) return;
      // LOG(row, col, a);
      mask ^= (1 << (a - 1));
      v1::matrix[row][col] = a;
      next();
      recursive_search();
      prev();
      mask ^= (1 << (a - 1));
      return;
    }

    if (row == 3) {
      int a = last_col_cell();
      if (a <= 0 || a > 16) return;
      if ((mask & (1 << (a - 1))) == 0) return;
      mask ^= (1 << (a - 1));
      v1::matrix[row][col] = a;
      next();
      recursive_search();
      prev();
      mask ^= (1 << (a - 1));
      return;
    }

    for (auto idx : std::views::iota(0, 16))
      if (mask & (1 << idx)) {
        v1::matrix[row][col] = idx + 1;
        mask ^= (1 << idx);
        next();
        recursive_search();
        prev();
        mask ^= (1 << idx);
      }
  }
} // namespace v2

int main() {
  if (0) {
    for (auto i : std::views::iota(1, 16 + 1))
      v1::stack.push_back(i);
    v1::recursive_search();
  }

  // v2::test_next_prev();

  v2::recursive_search();
}
