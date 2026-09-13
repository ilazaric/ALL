#pragma once

#include <ivl/logger>
#include <algorithm>
#include <cmath>
#include <span>
#include <vector>

std::vector<double> limiter(std::span<const double> data) {
  auto clipped_max = std::ranges::count_if(data, [](double pt) { return pt > 0.9; });
  auto clipped_min = std::ranges::count_if(data, [](double pt) { return pt < -0.9; });
  auto n = data.size();
  LOG(n);
  LOG(clipped_max, (double)clipped_max / (double)n);
  LOG(clipped_min, (double)clipped_min / (double)n);
  std::vector<double> forward(n), backward(n);
  double last = 0.0;
  for (size_t i = 0; i < n; ++i) {
    auto curr = std::abs(data[i]) * 1.1;
    if (last < curr) last = curr;
    forward[i] = last;
    last /= 1.001;
  }
  last = 0.0;
  for (size_t i = n - 1; i + 1; --i) {
    auto curr = std::abs(data[i]) * 1.1;
    if (last < curr) last = curr;
    backward[i] = last;
    last /= 1.001;
  }
  std::vector<double> ret(n);
  for (size_t i = 0; i < n; ++i) {
    auto curr = std::max(std::max(forward[i], backward[i]), 1.0);
    ret[i] = data[i] / curr;
  }
  return ret;
}
