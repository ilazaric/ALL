#pragma once

#include "eval"
#include "point"
#include <span>
#include <vector>

// 1 / |a-b|
// 1 / |a/|a|-b/|b||
// a = (ax,ay,az)
// b = (bx,by,bz)
// 1 / |(ax,ay,az)/(ax**2+ay**2+az**2)**0.5
//     -(bx,by,bz)/(bx**2+by**2+bz**2)**0.5|

// assumes points are normed
std::vector<point> gradient(std::span<const point> points) {
  // d [ ((a-b)^2)^-0.5 ]
  // -0.5 * ((a-b)^2)^-1.5 * d [ (a-b)^2 ]
  // -0.5 * ((a-b)^2)^-1.5 * d [ a.a - 2a.b + b.b ]
  // -0.5 * ((a-b)^2)^-1.5 * { 2a-2b, 2b-2a }
  // -1 * ((a-b)^2)^-1.5 * { a-b, b-a }
  std::vector<point> ret(points.size(), point{});
  for (std::size_t i = 0; i < points.size(); ++i)
    for (std::size_t j = 0; j < i; ++j) {
      auto c = 1 / distance(points[i], points[j]);
      // c = -c * c * c;
      auto foo = (points[i] - points[j]) * (c * c * c);
      ret[i] -= foo;
      ret[j] += foo;
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

std::vector<point> gradient2(std::span<const point> points) {
  std::vector<point> ret(points.size(), point{});
  for (std::size_t i = 0; i < points.size(); ++i)
    for (std::size_t j = 0; j < i; ++j) {
      auto a = points[i];
      auto b = points[j];
      // d [ ((a/an - b/bn)^2)^-0.5 ]
      // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ (a/an - b/bn)^2 ]
      // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ (a*ani - b*bni)^2 ]
      // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ (a*ani)^2 - 2 * (a*ani).(b*bni) + (b*bni)^2 ]
      // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ 1 - 2 * (a*ani).(b*bni) + 1 ]
      // ((a/an - b/bn)^2)^-1.5 * d [ (a*ani).(b*bni) ]
      // ((a/an - b/bn)^2)^-1.5 * d [ ani * bni * (a.b) ]
      // ((a/an - b/bn)^2)^-1.5 * ( (d ani) * bni * (a.b) + ani * (d bni) * (a.b) + ani * bni * d [ a.b ] )
      auto foo = [](point a) {
        double ani = 1.0 / std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
        point dani = (-1.0 * ani * ani * ani) * a;
        return std::tuple(ani, dani);
      };
      auto [ani, dani] = foo(a);
      auto [bni, dbni] = foo(b);
      // ((a/an - b/bn)^2)^-1.5
      double first = 1.0 / distance(a * ani, b * bni);
      first = first * first * first;

      ret[i] += first * (dani * bni + ani * bni * b);
      ret[j] += first * (dbni * ani + ani * bni * a);
    }
  return ret;
}

// // 1 / |a-b|
// std::vector<double> diff2(std::span<const point> points) {
//   std::vector<double> ret(points.size() * 3 * points.size() * 3, 0.0);
//   for (std::size_t i = 0; i < points.size(); ++i)
//     for (std::size_t j = 0; j < i; ++j) {
//     }
//   return ret;
// }

// backpropagation?

// a -> b -> c
// db/db = 1
// da/dc = da/db db/dc

// x,y,z ----> r
// dr/dx = ?
