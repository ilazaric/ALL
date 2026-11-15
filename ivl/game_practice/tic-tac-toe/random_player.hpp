#pragma once

#include <ivl/game-practice/tic-tac-toe/player>
#include <random>

namespace ivl::game_practice::tic_tac_toe {
struct random_player : player {
  bool taken[3][3]{};
  std::mt19937 rnd;

  explicit random_player(size_t seed) : rnd(seed) {}

  void start_game(bool is_first) override {}

  void notify(position p) override { taken[p.x][p.y] = true; }

  position random_pos() {
    std::uniform_int_distribution<> dist(0, 2);
    return position{dist(rnd), dist(rnd)};
  }

  position play() override {
  start:
    auto p = random_pos();
    if (taken[p.x][p.y]) goto start;
    taken[p.x][p.y] = true;
    return p;
  }
};
} // namespace ivl::game_practice::tic_tac_toe
