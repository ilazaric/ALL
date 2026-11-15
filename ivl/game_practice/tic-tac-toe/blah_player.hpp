#pragma once

#include <ivl/game-practice/tic-tac-toe/random_player>

namespace ivl::game_practice::tic_tac_toe {
struct blah_player : random_player {
  int turn = 0;

  using random_player::random_player;

  void notify(position p) override {
    ++turn;
    random_player::notify(p);
  }

  position play() override {
    ++turn;
    if (turn == 1 || turn == 2 && !taken[1][1]) {
      taken[1][1] = true;
      return {1, 1};
    }
    if (turn == 2) {
      taken[0][0] = true;
      return {0, 0};
    }
    return random_player::play();
  }
};
} // namespace ivl::game_practice::tic_tac_toe
