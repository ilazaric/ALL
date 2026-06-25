#include <ivl/logger>
#include "autodiff"
#include "eval"
#include "point"
#include <cmath>
#include <iomanip>
#include <print>

void show1(const autodiff_t& arg) {
  for (std::size_t r = 0; r < arg.diff_rank(); ++r) {
    std::println("rank: {}", r);
    for (std::size_t i = 0; i < arg.data[r].size(); ++i) std::println("data[r][{}] = {}", i, arg.data[r][i]);
  }
  std::println();
}

autodiff_t norm2(const autodiff_t& arg) { return dot(arg, arg); }

autodiff_t inverse(const autodiff_t& arg) {
  contract_assert(arg.output_size() == 1);
  if (arg.diff_rank() == 0) return arg;
  autodiff_t ret(arg.shape());
  double coef = -1.0 / arg.data[0][0];
  ret.data[0][0] = -coef;
  for (std::size_t r = 1; r < arg.diff_rank(); ++r) {
    for (std::size_t i = 0; i < arg.data[r].size(); ++i) {
      ret.data[r][i] = coef * dot_specific(arg, ret, r, i);
    }
  }
  return ret;
}

autodiff_t pow(autodiff_t arg, double e) {
  contract_assert(arg.output_size() == 1);
  // show1(arg);
  contract_assert(arg.data[0][0] > 1e-5);
  double free_coef = std::pow(arg.data[0][0], e);
  arg /= arg.data[0][0];
  arg.data[0][0] = 0.0;
  autodiff_t ret(arg.shape());
  for (std::size_t r = arg.diff_rank(); r; --r) {
    ret = dot(ret, arg);
    ret *= (e - (double)(r - 1)) / (double)r;
    ret.data[0][0] = 1.0;
  }
  ret *= free_coef;
  return ret;
}

autodiff_t norm(const autodiff_t& arg) { return pow(norm2(arg), 0.5); }

autodiff_t distance(const autodiff_t& left, const autodiff_t& right) { return norm(left - right); }

autodiff_t normed(const autodiff_t& left) { return scale(left, inverse(norm(left))); }

autodiff_t normed_distance(const autodiff_t& left, const autodiff_t& right) {
  return distance(normed(left), normed(right));
}

autodiff_t evaluate_basic(const autodiff_t& left, const autodiff_t& right) {
  return inverse(normed_distance(left, right));
}

autodiff_t subsample(const autodiff_t& arg, std::span<const std::size_t> idxs) {
  const auto [N, IM, DR] = arg.shape();
  const auto OM = idxs.size();
  for (auto idx : idxs) {
    0;
    contract_assert(idx < IM);
  }
  autodiff_t ret(N, OM, DR);
  for (std::size_t r = 0; r < DR; ++r) {
    const std::size_t NS = arg.data[r].size() / IM;
    for (std::size_t i = 0; i < NS; ++i) {
      for (std::size_t j = 0; j < OM; ++j) {
        ret.data[r][i * OM + j] = arg.data[r][i * IM + idxs[j]];
      }
    }
  }
  return ret;
}

// N points -> 3N input size
// output size = 1
// diff rank = 3
// problem is i want to have an (N,N,3) intermediate
// that is N^3 in size

autodiff_t evaluate(const autodiff_t& pts) {
  contract_assert(pts.input_size() == pts.output_size());
  contract_assert(pts.input_size() % 3 == 0);
  const std::size_t N = pts.input_size() / 3;
  std::vector<autodiff_t> parted_pts;
  for (std::size_t i = 0; i < N; ++i) {
    parted_pts.push_back(subsample(pts, std::array{i * 3, i * 3 + 1, i * 3 + 2}));
  }
  for (std::size_t i = 0; i < N; ++i) {
    parted_pts[i] = normed(parted_pts[i]);
  }
  autodiff_t ret(pts.input_size(), 1, pts.diff_rank());
  for (std::size_t i = 0; i < N; ++i)
    for (std::size_t j = 0; j < i; ++j) ret += inverse(distance(parted_pts[i], parted_pts[j]));
  return ret;
}

// std::vector<point> gradient(std::span<const point> points) {
//   autodiff_t foo(points.size() * 3, points.size() * 3, 2);
//   for (std::size_t i = 0; i < points.size(); ++i) {
//     foo.data[0][i * 3] = points[i].x;
//     foo.data[0][i * 3 + 1] = points[i].y;
//     foo.data[0][i * 3 + 2] = points[i].z;
//     foo.data[1][i * 3 * (points.size() * 3 + 1)] = 1.0;
//     foo.data[1][(i * 3 + 1) * (points.size() * 3 + 1)] = 1.0;
//     foo.data[1][(i * 3 + 2) * (points.size() * 3 + 1)] = 1.0;
//   }
//   auto ad = evaluate(foo);
//   contract_assert(ad.input_size() == points.size() * 3);
//   contract_assert(ad.output_size() == 1);
//   contract_assert(ad.diff_rank() == 2);
//   std::vector<point> ret(points.size());
//   for (std::size_t i = 0; i < points.size(); ++i) {
//     ret[i].x = ad.data[1][i * 3];
//     ret[i].y = ad.data[1][i * 3 + 1];
//     ret[i].z = ad.data[1][i * 3 + 2];
//   }
//   return ret;
// }

std::vector<point> gradient(std::span<const point> points) {
  std::vector<point> out(points.size(), point{});
  autodiff_t left(6, 3, 2);
  autodiff_t right(6, 3, 2);
  left.data[1][0 * 3 + 0] = 1.0;
  left.data[1][1 * 3 + 1] = 1.0;
  left.data[1][2 * 3 + 2] = 1.0;
  right.data[1][3 * 3 + 0] = 1.0;
  right.data[1][4 * 3 + 1] = 1.0;
  right.data[1][5 * 3 + 2] = 1.0;
  for (std::size_t i = 0; i < points.size(); ++i) {
    left.data[0][0] = points[i].x;
    left.data[0][1] = points[i].y;
    left.data[0][2] = points[i].z;
    // right.data[0][0] = points[i].x;
    // right.data[0][1] = points[i].y;
    // right.data[0][2] = points[i].z;
    for (std::size_t j = 0; j < i; ++j) {
      // left.data[0][3] = points[j].x;
      // left.data[0][4] = points[j].y;
      // left.data[0][5] = points[j].z;
      right.data[0][0] = points[j].x;
      right.data[0][1] = points[j].y;
      right.data[0][2] = points[j].z;
      // show1(left);
      // show1(right);
      // auto foo = inverse(distance(left, right));
      auto foo = evaluate_basic(left, right);
      contract_assert(foo.input_size() == 6);
      contract_assert(foo.output_size() == 1);
      contract_assert(foo.diff_rank() == 2);
      out[i].x += foo.data[1][0];
      out[i].y += foo.data[1][1];
      out[i].z += foo.data[1][2];
      out[j].x += foo.data[1][3];
      out[j].y += foo.data[1][4];
      out[j].z += foo.data[1][5];
    }
  }
  return out;
}

void normalize(std::span<point> points) {
  for (auto& p : points) p = p / norm(p);
}

std::vector<point> fixup(std::span<const point> points, double* last = nullptr) {
  auto g = gradient(points);
  for (int i = 0; i < points.size(); ++i) {
    g[i] -= points[i] * dot(g[i], points[i]);
  }
  auto mix = [&](double r) {
    std::vector copy(std::from_range, points);
    for (std::size_t i = 0; i < points.size(); ++i) copy[i] += -r * g[i];
    normalize(copy);
    return copy;
  };
  double lo = 0;
  double lo_e = evaluate_assume_normed(mix(lo));
  double hi = last ? *last : 1;
  double hi_e = evaluate_assume_normed(mix(hi));
  if (hi_e < lo_e - 1e-2) return mix(hi);
  while (true) {
    double nhi = hi * 1.5;
    double nhi_e = evaluate_assume_normed(mix(nhi));
    if (nhi_e > hi_e - 1e-6) break;
    hi = nhi;
    hi_e = nhi_e;
  }
  while ((hi - lo) > 1e-9) {
    double mid = (hi + lo) / 2;
    double mid_e = evaluate_assume_normed(mix(mid));
    if (lo_e < hi_e) hi = mid, hi_e = mid_e;
    else lo = mid, lo_e = mid_e;
  }
  // LOG(lo);
  if (last) *last = lo;
  return mix(lo);
}

bool try_gradient_fixup(std::span<point> points, double& ev, double* last = nullptr) {
  auto nxt = fixup(points, last);
  auto nxt_ev = evaluate(nxt);
  if (nxt_ev > ev - 1e-5) return false;
  for (std::size_t i = 0; i < points.size(); ++i) points[i] = nxt[i];
  ev = nxt_ev;
  normalize(points);
  return true;
}

void repeat_gradient_fixup(std::span<point> points, double& ev) {
  double last = 1.0;
  while (try_gradient_fixup(points, ev, &last)) LOG(ev, last);
}

double attempt(int n) {
  std::vector<point> points(n);
  for (int i = 0; i < n; ++i) points[i] = random_point();
  normalize(points);
  auto ev = evaluate(points);
  repeat_gradient_fixup(points, ev);
  LOG(n, ev);
  // auto g = gradient(points);
  // for (int i = 0; i < n; ++i) {
  //   g[i] -= points[i] * dot(g[i], points[i]);
  // }
  // LOG(std::format("{}", g));
  return ev;
}

int main() {
  std::cerr << std::setprecision(5) << std::fixed;

  if (1) {
    double mini = 1e100;
    for (int i = 0; i < 1; ++i) mini = std::min(mini, attempt(50));
    LOG(mini);
  }
}
