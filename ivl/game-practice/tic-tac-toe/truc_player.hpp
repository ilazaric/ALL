#pragma once

#include <ivl/game-practice/tic-tac-toe/player>

namespace ivl::game_practice::tic_tac_toe {
struct truc_player : player {
  bool mine[3][3]{};
  bool their[3][3]{};

  explicit truc_player(size_t) {}

  void start_game(bool is_first) override {}

  void notify(position p) override { their[p.x][p.y] = true; }

  bool available(int x, int y) { return !mine[x][y] && !their[x][y]; }

  position play_impl() {
    // we can win
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) {
        if (!available(i, j)) continue;
        if (mine[0][j] + mine[1][j] + mine[2][j] == 2) return {i, j};
        if (mine[i][0] + mine[i][1] + mine[i][2] == 2) return {i, j};
        if (i == j && mine[0][0] + mine[1][1] + mine[2][2] == 2) return {i, j};
        if (i + j == 2 && mine[0][2] + mine[1][1] + mine[2][0] == 2) return {i, j};
      }

    // they would win
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) {
        if (!available(i, j)) continue;
        if (their[0][j] + their[1][j] + their[2][j] == 2) return {i, j};
        if (their[i][0] + their[i][1] + their[i][2] == 2) return {i, j};
        if (i == j && their[0][0] + their[1][1] + their[2][2] == 2) return {i, j};
        if (i + j == 2 && their[0][2] + their[1][1] + their[2][0] == 2) return {i, j};
      }

    if (available(1, 1)) return {1, 1};
    if (available(0, 0)) return {0, 0};
    if (available(0, 2)) return {0, 2};
    if (available(2, 0)) return {2, 0};
    if (available(2, 2)) return {2, 2};
    if (available(1, 2)) return {1, 2};
    if (available(2, 1)) return {2, 1};
    if (available(1, 0)) return {1, 0};
    if (available(0, 1)) return {0, 1};
    throw;
  }

  position play() override {
    auto ret = play_impl();
    mine[ret.x][ret.y] = true;
    return ret;
  }
};
} // namespace ivl::game_practice::tic_tac_toe
