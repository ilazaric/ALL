#pragma once

#include "linalg"
#include <cmath>

namespace autodiff {
struct zero {
  shape_t is;
  shape_t os;
  shape_t input_shape() const { return is; }
  shape_t output_shape() const { return os; }
  mdarray operator()(const mdarray&) const { return mdarray(os); }
  zero diff() const { return zero{.is = is, .os = shape_t::linear_operator(is, os)}; }
};

struct constant {
  shape_t is;
  mdarray c;
  shape_t input_shape() const { return is; }
  shape_t output_shape() const { return c.shape; }
  mdarray operator()(const mdarray&) const { return c; }
  zero diff() const { return zero{.is = is, .os = c.shape}.diff(); }
};

struct id {
  shape_t e;
  shape_t input_shape() const { return e; }
  shape_t output_shape() const { return e; }
  mdarray operator()(const mdarray& arg) const { return arg; }
  auto diff() const {
    constant ret{.is = e, .c{shape_t::linear_operator(e, e)}};
    for (std::size_t i = 0; i < e.size(); ++i) ret.c.data[i * e.size() + i] = 1;
    return ret;
  }
};

struct scale {
  shape_t e;
  double c;
  shape_t input_shape() const { return e; }
  shape_t output_shape() const { return e; }
  mdarray operator()(mdarray arg) const {
    for (auto&& el : arg.data) el *= c;
    return arg;
  }
  auto diff() const {
    constant ret{.is = e, .c{shape_t::linear_operator(e, e)}};
    for (std::size_t i = 0; i < e.size(); ++i) ret.c.data[i * e.size() + i] = c;
    return ret;
  }
};

// V -> R
// f(v) = v.v
struct norm2 {
  shape_t input_shape() const { return shape_t::vector(3); }
  shape_t output_shape() const { return shape_t::field(); }
  mdarray operator()(const mdarray& arg) const {
    mdarray ret(shape_t::field());
    for (std::size_t i = 0; i < 3; ++i) {
      double v = arg.data[i];
      ret.data[0] += v * v;
    }
    return ret;
  }
  scale diff() const { return scale{.e = shape_t::vector(3), .c = 2}; }
};

template<typename A, typename B>
struct tenadd {
  A a;
  B b;
  shape_t input_shape() const { return a.input_shape(); }
  shape_t output_shape() const { return a.output_shape(); }
  mdarray operator()(const mdarray& arg) const {
    auto ar = a(arg);
    auto br = b(arg);
    for (std::size_t i = 0; i < ar.size(); ++i) ar.data[i] += br.data[i];
    return ar;
  }
  auto diff() const { return tenadd{a.diff(), b.diff()}; }
};

// x -> a(x) . b(x)
template<typename A, typename B>
struct tenmul {
  A a;
  B b;
  shape_t input_shape() const { return a.input_shape(); }
  shape_t output_shape() const {
    return shape_t::linear_operator(b.output_shape().linear_operator_from(), a.output_shape().linear_operator_to());
    // shape_t ret = b.output_shape();
    // ret.pop_back();
    // auto ret2 = a.output_shape();
    // ret.insert(ret.end(), ret2.begin() + 1, ret2.end());
    // return ret;
  }
  mdarray operator()(const mdarray& arg) const { return ::compose(b(arg), a(arg)); }
  auto diff() const {
    // Da(x) . b(x) + a(x) . Db(x) ???
    return tenadd{tenmul{a, b.diff()}, tenmul{a.diff(), b}};
  }
};

template<typename A, typename B>
struct comp {
  A a;
  B b;
  shape_t input_shape() const { return b.input_shape(); }
  shape_t output_shape() const { return a.output_shape(); }
  mdarray operator()(const mdarray& arg) const { return a(b(arg)); }
  auto diff() const {
    // D(a o b)(x) = Da(b(x)) . Db(x)
    return tenmul{comp{a.diff(), b}, b.diff()};
  }
};

struct delta {
  shape_t input_shape() const { return shape_t::vector(6); }
  shape_t output_shape() const { return shape_t::vector(3); }
  mdarray operator()(const mdarray& arg) const {
    mdarray ret(shape_t::vector(3));
    ret[0] = arg[0].extract_number() - arg[3].extract_number();
    ret[1] = arg[1].extract_number() - arg[4].extract_number();
    ret[2] = arg[2].extract_number() - arg[5].extract_number();
    return ret;
  }
  auto diff() const {
    constant ret{.is = shape_t::vector(6), .c{shape_t::linear_operator(shape_t::vector(6), shape_t::vector(3))}};
    ret.c[0][0] = 1;
    ret.c[1][1] = 1;
    ret.c[2][2] = 1;
    ret.c[3][0] = -1;
    ret.c[4][1] = -1;
    ret.c[5][2] = -1;
    return ret;
  }
};

// struct monomial {
//   double c;
//   double e;
//   shape_t input_shape() const { return {}; }
//   shape_t output_shape() const { return {}; }
//   mdarray operator()(mdarray arg) const {
//     arg = c * std::pow(arg.extract_number(), e);
//     return arg;
//   }
//   monomial diff() const { return monomial{.c = c * e, .e = e - 1}; }
// };
} // namespace autodiff
