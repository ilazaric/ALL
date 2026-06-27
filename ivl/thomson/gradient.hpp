#pragma once

#include "eval"
#include "point"
#include <ranges>
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

// gradient of actual function
std::vector<point> gradient2(std::span<const point> points) {
  std::vector<point> ret(points.size(), point{});
  auto foo = [](point a) {
    double ani = 1.0 / std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    point dani = (-1.0 * ani * ani * ani) * a;
    point anormed = ani * a;
    return std::tuple(ani, dani, anormed);
  };
  std::vector pre(std::from_range, points | std::views::transform(foo));
  for (std::size_t i = 0; i < points.size(); ++i) {
    auto [ani, dani, anormed] = pre[i]; // foo(points[i]);
    for (std::size_t j = 0; j < i; ++j) {
      // d [ ((a/an - b/bn)^2)^-0.5 ]
      // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ (a/an - b/bn)^2 ]
      // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ (a*ani - b*bni)^2 ]
      // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ (a*ani)^2 - 2 * (a*ani).(b*bni) + (b*bni)^2 ]
      // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ 1 - 2 * (a*ani).(b*bni) + 1 ]
      // ((a/an - b/bn)^2)^-1.5 * d [ (a*ani).(b*bni) ]
      // ((a/an - b/bn)^2)^-1.5 * d [ ani * bni * (a.b) ]
      // ((a/an - b/bn)^2)^-1.5 * ( (d ani) * bni * (a.b) + ani * (d bni) * (a.b) + ani * bni * d [ a.b ] )
      auto [bni, dbni, bnormed] = pre[j]; // foo(points[j]);
      // ((a/an - b/bn)^2)^-1.5
      double first = 1.0 / distance(anormed, bnormed);
      first = first * first * first;
      double dotab = dot(points[i], points[j]);
      ret[i] += first * (dani * bni * dotab + ani * bnormed);
      ret[j] += first * (dbni * ani * dotab + bni * anormed);
    }
  }
  return ret;
}

struct point2 {
  double data[3][3];
  using row_ref = double (&)[3];
  using row_cref = const double (&)[3];
  row_ref operator[](int i) {
    contract_assert(i >= 0 && i < 3);
    return data[i];
  }
  row_cref operator[](int i) const {
    contract_assert(i >= 0 && i < 3);
    return data[i];
  }

  point2& operator+=(const point2& o) {
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) data[i][j] += o.data[i][j];
    return *this;
  }
  point2& operator-=(const point2& o) {
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) data[i][j] -= o.data[i][j];
    return *this;
  }
  friend point2 operator+(point2 a, const point2& b) {
    a += b;
    return a;
  }
  friend point2 operator-(point2 a, const point2& b) {
    a -= b;
    return a;
  }

  point2& operator*=(double o) {
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) data[i][j] *= o;
    return *this;
  }
  friend point2 operator*(double o, point2 a) {
    a *= o;
    return a;
  }
  friend point2 operator*(point2 a, double o) {
    a *= o;
    return a;
  }
};

point2 ppmul(point a, point b) {
  point2 ret;
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j) ret[i][j] = a[i] * b[j];
  return ret;
}

point2 ppid() {
  point2 ret{};
  ret[0][0] = ret[1][1] = ret[2][2] = 1.0;
  return ret;
}

point2 pptranspose(point2 x) {
  std::swap(x[0][1], x[1][0]);
  std::swap(x[2][1], x[1][2]);
  std::swap(x[0][2], x[2][0]);
  return x;
}

// D2 of actual function
std::vector<point2> diff2(std::span<const point> points) {
  std::vector<point2> ret(points.size() * points.size(), point2{});
  // d [ ((a/an - b/bn)^2)^-0.5 ]
  // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ (a/an - b/bn)^2 ]
  // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ (a*ani - b*bni)^2 ]
  // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ (a*ani)^2 - 2 * (a*ani).(b*bni) + (b*bni)^2 ]
  // -0.5 * ((a/an - b/bn)^2)^-1.5 * d [ 1 - 2 * (a*ani).(b*bni) + 1 ]
  // ((a/an - b/bn)^2)^-1.5 * d [ (a*ani).(b*bni) ]
  // ((a/an - b/bn)^2)^-1.5 * d [ ani * bni * (a.b) ]
  // ((a/an - b/bn)^2)^-1.5 * ( (d ani) * bni * (a.b) + ani * (d bni) * (a.b) + ani * bni * d [ a.b ] )
  //
  // d2 [ ((a/an - b/bn)^2)^-1.5 * ( (d1 ani) * bni * (a.b) + ani * (d1 bni) * (a.b) + ani * bni * d1 [ a.b ] ) ]
  //
  // d2 [ ((a/an - b/bn)^2)^-1.5 ] * ( (d1 ani) * bni * (a.b) + ani * (d1 bni) * (a.b) + ani * bni * d1 [ a.b ] ) +
  // +   ((a/an - b/bn)^2)^-1.5 * d2 [ ( (d1 ani) * bni * (a.b) + ani * (d1 bni) * (a.b) + ani * bni * d1 [ a.b ] ) ]
  //
  // -1.5 * ((a/an - b/bn)^2)^-2.5 * d2 [ (a/an - b/bn)^2 ] *
  // ( (d1 ani) * bni * (a.b) + ani * (d1 bni) * (a.b) + ani * bni * d1 [ a.b ] ) +
  // ((a/an - b/bn)^2)^-1.5 * (
  //   d2 [ (d1 ani) * bni * (a.b) ] +
  //   d2 [ ani * (d1 bni) * (a.b) ] +
  //   d2 [ ani * bni * d1 [ a.b ] ]
  // )
  //
  // 3 * ((a/an - b/bn)^2)^-2.5 *
  // ( (d2 ani) * bni * (a.b) + ani * (d2 bni) * (a.b) + ani * bni * d2 [ a.b ] ) *
  // ( (d1 ani) * bni * (a.b) + ani * (d1 bni) * (a.b) + ani * bni * d1 [ a.b ] ) +
  // ((a/an - b/bn)^2)^-1.5 * (
  //   (dd ani) * bni * (a.b) +
  //   (d1 ani) * (d2 bni) * (a.b) +
  //   (d1 ani) * bni * d2 [ a.b ] +
  //   (d2 ani) * (d1 bni) * (a.b) +
  //   ani * (dd bni) * (a.b) +
  //   ani * (d1 bni) * d2 [ a.b ] +
  //   (d2 ani) * bni * d1 [ a.b ] +
  //   ani * (d2 bni) * d1 [ a.b ] +
  //   ani * bni * dd [ a.b ]
  // )
  for (std::size_t i = 0; i < points.size(); ++i) {
    for (std::size_t j = 0; j < i; ++j) {
      auto a = points[i];
      auto b = points[j];
      auto& daa = ret[i * points.size() + i];
      auto& dab = ret[i * points.size() + j];
      auto& dba = ret[j * points.size() + i];
      auto& dbb = ret[j * points.size() + j];
      auto foo = [](point a) {
        double ani = 1.0 / std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
        point dani = (-1.0 * ani * ani * ani) * a;
        point2 ddani = -3.0 * ani * ani * ppmul(a, dani) - ani * ani * ani * ppid();
        point anormed = ani * a;
        return std::tuple(ani, dani, ddani, anormed);
      };
      auto [ani, dani, ddani, anormed] = foo(a);
      auto [bni, dbni, ddbni, bnormed] = foo(b);
      double rdist = 1.0 / distance(anormed, bnormed);
      double dotab = dot(a, b);
      {
        double first = 3 * rdist * rdist * rdist * rdist * rdist;
        for (int d1 = 0; d1 < 3; ++d1)
          for (int d2 = 0; d2 < 3; ++d2) {
            double& aa = daa[d1][d2];
            double& ab = dab[d1][d2];
            double& ba = dba[d1][d2];
            double& bb = dbb[d1][d2];
            double fa = dani[d1] * bni * dotab + ani * bni * b[d1];
            double fb = dbni[d1] * ani * dotab + ani * bni * a[d1];
            double la = dani[d2] * bni * dotab + ani * bni * b[d2];
            double lb = dbni[d2] * ani * dotab + ani * bni * a[d2];
            aa += first * fa * la;
            ab += first * fa * lb;
            ba += first * fb * la;
            bb += first * fb * lb;
          }
      }
      {
        double first = rdist * rdist * rdist;
        //   (dd ani) * bni * (a.b)
        daa += first * ddani * bni * dotab;
        //   (d1 ani) * (d2 bni) * (a.b)
        dab += first * ppmul(dani, dbni) * dotab;
        //   (d1 ani) * bni * d2 [ a.b ]
        daa += first * ppmul(dani, b) * bni;
        dab += first * ppmul(dani, a) * bni;
        //   (d2 ani) * (d1 bni) * (a.b)
        dba += first * ppmul(dbni, dani) * dotab;
        //   ani * (dd bni) * (a.b)
        dbb += first * ddbni * ani * dotab;
        //   ani * (d1 bni) * d2 [ a.b ]
        dba += first * ppmul(dbni, b) * ani;
        dbb += first * ppmul(dbni, a) * ani;
        //   (d2 ani) * bni * d1 [ a.b ]
        daa += first * ppmul(b, dani) * bni;
        dba += first * ppmul(a, dani) * bni;
        //   ani * (d2 bni) * d1 [ a.b ]
        dab += first * ppmul(b, dbni) * ani;
        dbb += first * ppmul(a, dbni) * ani;
        //   ani * bni * dd [ a.b ]
        dab += first * ppid() * ani * bni;
        dba += first * ppid() * ani * bni;
      }
    }
  }
  for (std::size_t i = 0; i < points.size(); ++i)
    for (std::size_t j = 0; j < points.size(); ++j) {
      auto delta = ret[i * points.size() + j] - pptranspose(ret[j * points.size() + i]);
      for (int x = 0; x < 3; ++x)
        for (int y = 0; y < 3; ++y) {
          0;
          contract_assert(abs(delta[x][y]) < 1e-5);
        }
    }
  return ret;
}
