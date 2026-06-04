#include <ivl/logger>
#include "point"
#include <format>
#include <span>
#include <vector>

/*
  f : A -> B
  Df : A -> L(A, B)

  D(f o g)(x) = (Df o g)(x) * Dg(x)
  g : A -> B
  f : B -> C
  D(f o g) : A -> L(A, C)
  Df : B -> L(B, C)
  Df o g : A -> L(B, C)
  (Df o g)(x) : L(B, C)
  Dg(x) : L(A, B)
  (Df o g)(x) * Dg(x) : L(B, C) * L(A, B) = L(A, C)


  (a, b) -> 1 / |a-b|
  f o g (a,b)
  f(x) = x^-0.5
  g(a,b) = (a-b)^2
  Dg(a,b) =? 2[a-b,b-a]
  Df(x) = -0.5 * x^-1.5
  D(fog)(a,b) = -0.5 * (a-b)^2^-1.5 * 2[a-b,b-a]
  = -|a-b|^-3 * [a-b,b-a]

  1/sqrt((x-t)^2+u)
  ((x-t)^2+u)^-0.5
  -0.5 * ((x-t)^2+u)^-1.5 * 2(x-t)
 */

// double
// std::array<double, N>
// std::vector<double>

// struct scale {
//   double coef;
//   auto operator()(const auto& input) const {
//     return input * coef;
//   }
// };

/*
  f : sum 1/|p-ti| = sum (p-ti)^2 ^-0.5
  df/dx : sum -0.5 * (p-ti)^2 ^-1.5 * 2(x-tix)
  Df : sum -|p-ti|^-3 * (p-ti)
 */

double evaluate(point a, point b) { return 1 / normed_distance(a, b); }

double evaluate(std::span<const point> points) {
  double ret = 0;
  for (std::size_t i = 0; i < points.size(); ++i)
    for (std::size_t j = 0; j < i; ++j) ret += evaluate(points[i], points[j]);
  return ret;
}

// assumes points are normed
std::vector<point> gradient(std::span<const point> points) {
  std::vector<point> ret(points.size(), point{});
  for (std::size_t i = 0; i < points.size(); ++i)
    for (std::size_t j = 0; j < i; ++j) {
      auto c = distance(points[i], points[j]);
      c = -c * c * c;
      ret[i] += (points[i] - points[j]) * c;
      ret[j] += (points[j] - points[i]) * c;
    }
  return ret;
}

std::vector<point> fixup(std::span<const point> points) {
  auto g = gradient(points);
  LOG(std::format("{}", points));
  LOG(std::format("{}", g));
  auto mix = [&](double r) {
    std::vector copy(std::from_range, points);
    for (std::size_t i = 0; i < points.size(); ++i) copy[i] += -r * g[i];
    // LOG(std::format("{}", copy));
    return copy;
  };
  double lo = -1;
  double lo_e = evaluate(mix(lo));
  double hi = 1;
  double hi_e = evaluate(mix(hi));
  while (true) {
    double nhi = hi * 2;
    double nhi_e = evaluate(mix(nhi));
    if (nhi_e > hi_e - 1e-5) break;
    hi = nhi;
    hi_e = nhi_e;
  }
  while (true) {
    double nlo = lo * 2;
    double nlo_e = evaluate(mix(nlo));
    if (nlo_e > lo_e - 1e-5) break;
    lo = nlo;
    lo_e = nlo_e;
  }
  LOG(lo, lo_e);
  LOG(hi, hi_e);
  while ((hi - lo) > 1e-9) {
    double mid = (hi + lo) / 2;
    double mid_e = evaluate(mix(mid));
    LOG(mid, mid_e);
    if (lo_e < hi_e) hi = mid, hi_e = mid_e;
    else lo = mid, lo_e = mid_e;
  }
  LOG(lo, lo_e);
  return mix(lo);
}

int ivl_main() {
  int n = 6;
  std::vector<point> points;
  for (int i = 0; i < n; ++i) {
    auto p = random_point();
    points.push_back(p / norm(p));
  }
  auto ev = evaluate(points);
  LOG(ev);
  while (true) {
    auto nxt = fixup(points);
    auto nxt_ev = evaluate(nxt);
    LOG(nxt_ev);
    if (nxt_ev > ev - 1e-5) break;
    ev = nxt_ev;
    points = nxt;
    for (auto& p : points) p = p / norm(p);
  }
  {
    contract_assert(n == 6);
    std::vector<point> correct(6, point{});
    correct[0].x = +1;
    correct[1].x = -1;
    correct[2].y = +1;
    correct[3].y = -1;
    correct[4].z = +1;
    correct[5].z = -1;
    LOG(evaluate(correct));
    // LOG(std::format("{}", gradient(correct)));
  }
  return 0;
}
