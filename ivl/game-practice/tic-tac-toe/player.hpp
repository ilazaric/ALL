#pragma once

namespace ivl::game_practice::tic_tac_toe {
struct position {
  int x;
  int y;
};

struct player {
  virtual void start_game(bool is_first) = 0;
  
  // opponent played at that position
  virtual void notify(position) = 0;
  
  // i want to place move here
  virtual position play() = 0;
};
} // namespace ivl::game_practice::tic_tac_toe
