#pragma once

#include <random>
#include <span>
#include <vector>

namespace ivl {
template<typename T>
struct pseudo_boolean_optimize_t {
  size_t n;
  size_t rep;
  T& callable;
  size_t seed = 1337;
  std::mt19937 gen{seed};
  std::vector<bool> curr = std::vector<bool>(n, false);
  std::vector<bool> best = std::vector<bool>(n, false);
  decltype(callable(std::span<const bool>{})) best_eval{run(best)};

  void mutate_flip(double p) {
    std::bernoulli_distribution dist(p);
    for (auto&& el : curr)
      if (dist(gen)) el = !el;
  }

  void mutate_flip_2() {
    std::uniform_int_distribution<size_t> dist(0, n);
    auto lo = dist(gen);
    auto hi = dist(gen);
    if (lo == hi) return;
    if (lo > hi) std::swap(lo, hi);
    for (size_t i = lo; i < hi; ++i) curr[i] = !curr[i];
  }

  void mutate_set(bool v) {
    std::uniform_int_distribution<size_t> dist(0, n);
    auto lo = dist(gen);
    auto hi = dist(gen);
    if (lo == hi) return;
    if (lo > hi) std::swap(lo, hi);
    for (size_t i = lo; i < hi; ++i) curr[i] = v;
  }

  void mutate_shift(double p) {
    std::bernoulli_distribution dist(p);
    for (size_t i = 0; i + i < curr.size(); ++i)
      if (dist(gen)) std::swap(curr[i], curr[i + 1]);
  }

  void mutate_rotate(bool dir) {
    std::uniform_int_distribution<size_t> dist(0, n);
    auto lo = dist(gen);
    auto hi = dist(gen);
    if (lo == hi) return;
    if (lo > hi) std::swap(lo, hi);
    std::rotate(curr.begin() + lo, curr.begin() + (dir ? lo + 1 : hi - 1), curr.begin() + hi);
  }

  auto run(const std::vector<bool>& vec) { return callable(std::span<const bool>(vec.data(), n)); }
  auto run() { return run(curr); }

  bool try_run() {
    auto eval = run();
    if (eval > best_eval) {
      best = curr;
      best_eval = eval;
      return true;
    } else {
      curr = best;
      return false;
    }
  }

  void big_run() {
    double flip_p = 0.5;
    double shift_p = 0.5;
    for (size_t i = 0; i < rep; ++i) {
      mutate_flip(flip_p);
      if (!try_run()) flip_p /= 1.005;
      mutate_flip_2();
      try_run();
      mutate_set(true);
      try_run();
      mutate_set(false);
      try_run();
      mutate_shift(shift_p);
      if (!try_run()) shift_p /= 1.005;
      mutate_rotate(true);
      try_run();
      mutate_rotate(false);
      try_run();
    }
    for (size_t i = 0; i < n; ++i) {
      curr[i] = !curr[i];
      try_run();
    }
  }

  void big_run2() {
    best.clear();
    for (size_t i = 0; i < n; ++i) {
      auto c = best;
      auto try_at = [&](size_t pos, bool value) {
        curr = c;
        curr.insert(curr.begin() + pos, value);
        try_run();
      };
      best.push_back(false);
      best_eval = run(best);
      try_at(0, true);
      try_at(0, false);
      try_at(i, true);
      try_at(i, false);
      std::uniform_int_distribution<size_t> dist(0, i);
      for (size_t j = 0; j < 10; ++j) {
        try_at(dist(gen), true);
        try_at(dist(gen), false);
      }
    }
  }

  auto done() { return std::pair{std::move(best), std::move(best_eval)}; }

  auto operator()() {
    big_run();
    return done();
  }
};

auto pseudo_boolean_optimize(size_t n, size_t rep, auto&& callable) {
  return pseudo_boolean_optimize_t{n, rep, callable}();
}
} // namespace ivl
