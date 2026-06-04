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
  we want Df to be perpendicular to surface
  equiv. to: Df(p) || p
  p || sum -|p-ti|^-3 * (p-ti)
  p || sum |p-ti|^-3 * ti

  |a-b| = sqrt((ax-bx)^2 + (ay-by)^2 + (az-bz)^2)
 */

double evaluate(point a, point b) { return 1 / normed_distance(a, b); }

double evaluate(std::span<const point> points) {
  double ret = 0;
  for (std::size_t i = 0; i < points.size(); ++i)
    for (std::size_t j = 0; j < i; ++j) ret += evaluate(points[i], points[j]);
  return ret;
}

void normalize(std::span<point> points) {
  for (auto& p : points) p = p / norm(p);
}

// struct construction {
//   inline static constexpr double UNINIT = -1;
//   std::vector<point> points;
//   double eval = UNINIT;
//   double evaluate() {
//     if (eval == UNINIT) eval = ::evaluate(points);
//     return eval;
//   }
//   void reset_eval() { eval = UNINIT; }
// };

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
  // LOG(std::format("{}", points));
  // LOG(std::format("{}", g));
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
  // LOG(lo, lo_e);
  // LOG(hi, hi_e);
  while ((hi - lo) > 1e-9) {
    double mid = (hi + lo) / 2;
    double mid_e = evaluate(mix(mid));
    // LOG(mid, mid_e);
    if (lo_e < hi_e) hi = mid, hi_e = mid_e;
    else lo = mid, lo_e = mid_e;
  }
  // LOG(lo, lo_e);
  return mix(lo);
}

bool try_gradient_fixup(std::span<point> points, double& ev) {
  auto nxt = fixup(points);
  auto nxt_ev = evaluate(nxt);
  if (nxt_ev > ev - 1e-5) return false;
  for (std::size_t i = 0; i < points.size(); ++i) points[i] = nxt[i];
  ev = nxt_ev;
  normalize(points);
  return true;
}

void repeat_gradient_fixup(std::span<point> points, double& ev) { while (try_gradient_fixup(points, ev)); }

double attempt(int n) {
  std::vector<point> points(n);
  for (int i = 0; i < n; ++i) points[i] = random_point();
  normalize(points);
  auto ev = evaluate(points);
  repeat_gradient_fixup(points, ev);
  LOG(n, ev);
  return ev;
}

template<typename = void>
void perturb(std::span<point> points, double r) {
  static std::mt19937 gen(40101041);
  for (auto& [... els] : points) ((els += std::uniform_real_distribution(-r, r)(gen)), ...);
}

double attempt2(int n) {
  std::vector<point> points(n);
  for (int i = 0; i < n; ++i) points[i] = random_point();
  normalize(points);
  auto ev = evaluate(points);
  auto best_ev = ev;
  for (int i = 0; i < 10; ++i) {
    repeat_gradient_fixup(points, ev);
    if (ev < best_ev) best_ev = ev;
    perturb(points, 0.1);
    normalize(points);
    ev = evaluate(points);
  }
  LOG(n, best_ev);
  return best_ev;
}

int ivl_main() {
  {
    double mini = 1e100;
    for (int i = 0; i < 10; ++i) mini = std::min(mini, attempt(100));
    LOG(mini);
  }
  {
    double mini = 1e100;
    for (int i = 0; i < 10; ++i) mini = std::min(mini, attempt2(100));
    LOG(mini);
  }
  if (0) {
    int n = 100;
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
  }
  return 0;
}
