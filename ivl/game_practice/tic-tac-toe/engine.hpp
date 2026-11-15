#pragma once

#include <ivl/game-practice/tic-tac-toe/player>
#include <cstdint>
#include <utility>

namespace ivl::game_practice::tic_tac_toe {
enum class cell : uint8_t {
  EMPTY,
  X,
  O,
};

enum class outcome : uint8_t {
  NOT_FINISHED,
  FIRST,
  DRAW,
  SECOND,
};

struct board {
  cell cells[3][3];

  board() {
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) cells[i][j] = cell::EMPTY;
  }

  outcome finished() const {
    auto check = [&](cell a, cell b, cell c) {
      if (a == cell::EMPTY || a != b || a != c) return outcome::NOT_FINISHED;
      return a == cell::X ? outcome::FIRST : outcome::SECOND;
    };

    for (int i = 0; i < 3; ++i) {
      if (auto c = check(cells[i][0], cells[i][1], cells[i][2]); c != outcome::NOT_FINISHED) return c;
      if (auto c = check(cells[0][i], cells[1][i], cells[2][i]); c != outcome::NOT_FINISHED) return c;
    }

    if (auto c = check(cells[0][0], cells[1][1], cells[2][2]); c != outcome::NOT_FINISHED) return c;
    if (auto c = check(cells[0][2], cells[1][1], cells[2][0]); c != outcome::NOT_FINISHED) return c;

    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        if (cells[i][j] == cell::EMPTY) return outcome::NOT_FINISHED;

    return outcome::DRAW;
  }
};

outcome play(player& first_player, player& second_player) {
  first_player.start_game(true);
  second_player.start_game(false);

  board board;

  struct player_info {
    player* p;
    cell c;
    outcome out;
  };

  player_info current{&first_player, cell::X, outcome::FIRST};
  player_info next{&second_player, cell::O, outcome::SECOND};

  while (board.finished() == outcome::NOT_FINISHED) {
    auto pos = current.p->play();
    if (pos.x < 0 || pos.x >= 3) return next.out;
    if (pos.y < 0 || pos.y >= 3) return next.out;
    if (board.cells[pos.x][pos.y] != cell::EMPTY) return next.out;
    board.cells[pos.x][pos.y] = current.c;
    next.p->notify(pos);
    std::swap(current, next);
  }

  return board.finished();
}
} // namespace ivl::game_practice::tic_tac_toe
