#pragma once

#include <ivl/game-practice/tic-tac-toe/random_player>

namespace ivl::game_practice::tic_tac_toe {
struct random_center_player : random_player {
  using random_player::random_player;

  position play() override {
    if (!taken[1][1]) {
      taken[1][1] = true;
      return {1, 1};
    }
    return random_player::play();
  }
};
} // namespace ivl::game_practice::tic_tac_toe
