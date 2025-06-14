#pragma once

namespace ivl::games::chess {

  enum class struct Cell {
    std::int8_t data;

    bool empty() const {return data == 0;}
  bool white() const {
    return data > 0;
  }
  bool black() const {
    return data < 0;
  }

}; // namespace ivl::games::chess

struct Board {
  std::array<std::array<Cell, 8>, 8> data;
};

} // namespace ivl::games::chess
