#pragma once

#include <cmath>
#include <random>
#include <format>

struct point {
  double x, y, z;
};

template<>
struct std::formatter<point, char> {
  constexpr auto parse(auto& ctx) {
    if (ctx.begin() != ctx.end() && *ctx.begin() == '}') throw;
    return ctx.begin();
  }
  auto format(point p, auto& ctx) const { return std::format_to(ctx.out(), "({:.5f},{:.5f},{:.5f})", p.x, p.y, p.z); }
};

template<typename = void>
point operator-(point a, point b) {
  auto [... as] = a;
  auto [... bs] = b;
  return point{as - bs...};
}

template<typename = void>
point operator+(point a, point b) {
  auto [... as] = a;
  auto [... bs] = b;
  return point{as + bs...};
}

template<typename = void>
point operator/(point a, double r) {
  // TODO: maybe contract_assert(r > eps)
  auto [... as] = a;
  return point{as / r...};
}

template<typename = void>
point operator*(point a, double r) {
  auto [... as] = a;
  return point{as * r...};
}

template<typename = void>
point operator*(double r, point a) {
  auto [... as] = a;
  return point{as * r...};
}

point& operator+=(point& a, point b) { return a = a + b; }
point& operator-=(point& a, point b) { return a = a - b; }

template<typename = void>
double dot(point a, point b) {
  auto [... as] = a;
  auto [... bs] = b;
  return ((as * bs) + ... + 0);
}

double norm(point a) { return std::sqrt(dot(a, a)); }

double distance(point a, point b) { return norm(a - b); }

double normed_distance(point a, point b) { return distance(a / norm(a), b / norm(b)); }

template<typename = void>
point random_point() {
  static std::mt19937 gen(1337);
  point ret;
  auto& [... els] = ret;
  ((els = std::uniform_real_distribution(-1.0, 1.0)(gen)), ...);
  return ret;
}
