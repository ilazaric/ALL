#pragma once

#include <ivl/command_line_argument_parsing/implicit_exposed>
#include <ivl/logger>
#include <algorithm>
#include <cmath>
#include <span>
#include <vector>

std::vector<double> limiter(std::span<const double> data) {
  const double limit = implicit_value("limiter_limit", 0.9);
  const double decay = implicit_value("limiter_decay", 1.001);
  LOG(limit);
  LOG(decay);
  auto clipped_max = std::ranges::count_if(data, [&](double pt) { return pt > limit; });
  auto clipped_min = std::ranges::count_if(data, [&](double pt) { return pt < -limit; });
  auto n = data.size();
  LOG(n);
  LOG(clipped_max, (double)clipped_max / (double)n);
  LOG(clipped_min, (double)clipped_min / (double)n);
  LOG(std::ranges::max(data));
  LOG(std::ranges::min(data));
  std::vector<double> forward(n), backward(n);
  double last = 0.0;
  for (size_t i = 0; i < n; ++i) {
    auto curr = std::abs(data[i]) / limit;
    if (last < curr) last = curr;
    forward[i] = last;
    last /= decay;
  }
  last = 0.0;
  for (size_t i = n - 1; i + 1; --i) {
    auto curr = std::abs(data[i]) / limit;
    if (last < curr) last = curr;
    backward[i] = last;
    last /= decay;
  }
  std::vector<double> ret(n);
  for (size_t i = 0; i < n; ++i) {
    auto curr = std::max(std::max(forward[i], backward[i]), 1.0);
    ret[i] = data[i] / curr;
  }
  return ret;
}
