#pragma once

#include <raylib/raylib.h>
#include <cmath>
#include <vector>

template<typename T>
std::vector<T> len_binned(const std::vector<T>& data, size_t len) {
  contract_assert(data.size() % len == 0);
  std::vector<T> ret(data.size() / len, T{});
  for (size_t i = 0; i < data.size(); ++i) ret[i / len] += data[i];
  return ret;
}

std::vector<Vector2> x_binned(const std::vector<Vector2>& data, double len) {
  std::vector<Vector2> ret;
  double last_i = 0;
  double last_ys = 0;
  size_t last_cnt = 0;
  for (auto [x, y] : data) {
    auto i = std::round(x / len);
    if (i != last_i) {
      if (last_cnt != 0) ret.emplace_back(last_i * len, last_ys / (double)last_cnt);
      last_i = i;
      last_ys = 0;
      last_cnt = 0;
    }
    last_ys += y;
    ++last_cnt;
  }
  if (last_cnt != 0) ret.emplace_back(last_i * len, last_ys / (double)last_cnt);
  return ret;
}
