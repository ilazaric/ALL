#pragma once

#include "point"
#include <span>

double evaluate(point a, point b) { return 1 / normed_distance(a, b); }

double evaluate(std::span<const point> points) {
  double ret = 0;
  for (std::size_t i = 0; i < points.size(); ++i)
    for (std::size_t j = 0; j < i; ++j) ret += evaluate(points[i], points[j]);
  return ret;
}

double evaluate_for(std::span<const point> points, std::size_t j) {
  double ret = 0;
  for (std::size_t i = 0; i < points.size(); ++i)
    if (i != j) ret += evaluate(points[i], points[j]);
  return ret;
}
