#include <ivl/game-practice/tic-tac-toe/engine>
#include <ivl/game-practice/tic-tac-toe/random_center_player>
#include <ivl/game-practice/tic-tac-toe/random_player>
#include <ivl/game-practice/tic-tac-toe/blah_player>
#include <ivl/game-practice/tic-tac-toe/truc_player>
#include <meta>
#include <print>

struct biased_matrix_outcome {
  size_t total;
  size_t outcomes[3];
};

struct unbiased_matrix_outcome {
  biased_matrix_outcome outcomes[2];
};

template<typename P1, typename P2>
biased_matrix_outcome biased_matrix_battle(size_t seednum) {
  biased_matrix_outcome ret;
  ret.total = seednum * seednum;
  for (size_t i = 0; i < seednum; ++i)
    for (size_t j = 0; j < seednum; ++j) {
      P1 p1(i);
      P2 p2(j);
      ++ret.outcomes[(uint8_t)ivl::game_practice::tic_tac_toe::play(p1, p2) - 1];
    }
  return ret;
}

template<typename P1, typename P2>
unbiased_matrix_outcome matrix_battle(size_t seednum) {
  return {biased_matrix_battle<P1, P2>(seednum), biased_matrix_battle<P2, P1>(seednum)};
}

std::string trimmed(std::string s) {
  std::string rem = "ivl::game_practice::tic_tac_toe::";
  while (true) {
    auto loc = s.find(rem);
    if (loc == std::string::npos) return s;
    s = s.substr(0, loc) + s.substr(loc + rem.size());
  }
}

template<typename P1, typename P2>
double sum_battle(size_t seednum) {
  using enum ivl::game_practice::tic_tac_toe::outcome;

  int64_t ret = 0;
  for (size_t i = 0; i < seednum; ++i)
    for (size_t j = 0; j < seednum; ++j) {
      P1 p1(i);
      P2 p2(j);
      auto outcome = ivl::game_practice::tic_tac_toe::play(p1, p2);
      if (outcome == FIRST) ++ret;
      if (outcome == SECOND) --ret;
    }
  for (size_t i = 0; i < seednum; ++i)
    for (size_t j = 0; j < seednum; ++j) {
      P1 p1(i);
      P2 p2(j);
      auto outcome = ivl::game_practice::tic_tac_toe::play(p2, p1);
      if (outcome == FIRST) --ret;
      if (outcome == SECOND) ++ret;
    }
  return (double)ret / seednum / seednum / 2 * 100;
}

template<typename... Ps>
void table(size_t seednum) {
  std::array<std::array<std::string, sizeof...(Ps) + 1>, sizeof...(Ps) + 1> texts;
  texts[0][0] = std::format("seednum={}", seednum);
  template for (int i = 0; constexpr auto p : {^^Ps...}) {
    ++i;
    texts[0][i] = texts[i][0] = trimmed(std::string(display_string_of(p)));
  }

  template for (int i = 0; constexpr auto p1 : {^^Ps...}) {
    ++i;
    template for (int j = 0; constexpr auto p2 : {^^Ps...}) {
      ++j;
      texts[i][j] = std::format("{:.5f}", sum_battle<typename[:p1:], typename[:p2:]>(seednum));
    }
  }

  std::array<size_t, sizeof...(Ps) + 1> widths;
  for (size_t i = 0; i < texts.size(); ++i)
    for (size_t j = 0; j < texts[i].size(); ++j) widths[j] = std::max(widths[j], texts[i][j].size() + 2);

  for (size_t i = 0; i < texts.size(); ++i) {
    if (i) {
      for (size_t j = 0; j < texts[i].size(); ++j) {
        if (j) std::print("+");
        std::print("{:-<{}}", "", widths[j]);
      }
      std::println();
    }
    for (size_t j = 0; j < texts[i].size(); ++j) {
      if (j) std::print("|");
      std::print("{: ^{}}", texts[i][j], widths[j]);
    }
    std::println();
  }
}

void describe(const unbiased_matrix_outcome& ret, std::string_view p1, std::string_view p2) {
  std::println("{} first, {} second", p1, p2);
  std::println(
    "first won: {}, draws: {}, second won: {}", ret.outcomes[0].outcomes[0], ret.outcomes[0].outcomes[1],
    ret.outcomes[0].outcomes[2]
  );
  std::println();
  std::println("{} first, {} second", p2, p1);
  std::println(
    "first won: {}, draws: {}, second won: {}", ret.outcomes[1].outcomes[0], ret.outcomes[1].outcomes[1],
    ret.outcomes[1].outcomes[2]
  );
  std::println();
  std::println("cummulative");
  std::println(
    "{} won: {}, draws: {}, {} won: {}", p1, ret.outcomes[0].outcomes[0] + ret.outcomes[1].outcomes[2],
    ret.outcomes[0].outcomes[1] + ret.outcomes[1].outcomes[1], p2,
    ret.outcomes[0].outcomes[2] + ret.outcomes[1].outcomes[0]
  );
  std::println();
  auto summary = (double)(ret.outcomes[0].outcomes[0] + ret.outcomes[1].outcomes[2]) -
                 (double)(ret.outcomes[0].outcomes[2] + ret.outcomes[1].outcomes[0]);
  summary /= ret.outcomes[0].total + ret.outcomes[1].total;
  std::println("summary of {} vs {}: {:.2f}", p1, p2, summary * 100);
}

int main() {
  using namespace ivl::game_practice::tic_tac_toe;

  // auto ret = matrix_battle<random_player, random_center_player>(100);
  // describe(ret, "random_player", "random_center_player");
  // std::println();

  table<random_player, random_center_player, blah_player, truc_player>(100);

  return 0;
}
