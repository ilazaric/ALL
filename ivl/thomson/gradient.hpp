#pragma once

#include "eval"
#include "point"
#include <span>
#include <vector>

// assumes points are normed
std::vector<point> gradient(std::span<const point> points) {
  std::vector<point> ret(points.size(), point{});
  for (std::size_t i = 0; i < points.size(); ++i)
    for (std::size_t j = 0; j < i; ++j) {
      auto c = 1 / distance(points[i], points[j]);
      c = -c * c * c;
      ret[i] += (points[i] - points[j]) * c;
      ret[j] += (points[j] - points[i]) * c;
    }
  return ret;
}

// assumes points are normed
point gradient_for(std::span<const point> points, std::size_t j) {
  point ret{};
  for (std::size_t i = 0; i < points.size(); ++i)
    if (i != j) {
      auto c = 1 / distance(points[i], points[j]);
      c = -c * c * c;
      ret += (points[j] - points[i]) * c;
    }
  return ret;
}
