#pragma once

#include <array>
#include <cstdint>

namespace ivl::games::chess {
struct Cell {
  std::int8_t data;

  bool empty() const { return data == 0; }
  bool white() const { return data > 0; }
  bool black() const { return data < 0; }
};

struct Board {
  std::array<std::array<Cell, 8>, 8> data;
};
} // namespace ivl::games::chess
