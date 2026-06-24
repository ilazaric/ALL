#pragma once

#include <cstdint>
#include <span>
#include <tuple>
#include <vector>

struct autodiff_t {
  std::size_t in_size;
  std::size_t out_size;
  std::vector<std::vector<double>> data;
  // invariant: data[i].size() == out_size * in_size^i
  // shape: R^in -> R^in -> ... -> R^out
  // layout: data[i][a][b][c] is always contiguous

  autodiff_t(std::size_t in_size, std::size_t out_size, std::size_t dr)
      : in_size(in_size), out_size(out_size), data(dr) {
    std::size_t curr = out_size;
    for (std::size_t i = 0; i < diff_rank(); ++i) {
      data[i].resize(curr, 0.0);
      curr *= in_size;
    }
  }

  using shape_t = std::tuple<std::size_t, std::size_t, std::size_t>;

  explicit autodiff_t(shape_t shape) : autodiff_t(std::get<0>(shape), std::get<1>(shape), std::get<2>(shape)) {}

  std::size_t input_size() const { return in_size; }
  std::size_t output_size() const { return out_size; }
  std::size_t diff_rank() const { return data.size(); }
  shape_t shape() const { return std::tuple{input_size(), output_size(), diff_rank()}; }

  void decompose_index(std::size_t index, std::span<std::size_t> in, std::size_t& out) const {
    out = index % out_size;
    index /= out_size;
    for (std::size_t i = in.size() - 1; i + 1; --i) {
      in[i] = index % in_size;
      index /= in_size;
    }
    contract_assert(index == 0);
  }

  void verify_symmetry() const {
    for (std::size_t rank = 2, foo = 1; rank < diff_rank(); ++rank, foo *= input_size())
      for (std::size_t i0 = 0; i0 < input_size(); ++i0)
        for (std::size_t i1 = 0; i1 < input_size(); ++i1)
          for (std::size_t ir = 0; ir < foo; ++ir) {
            std::size_t i = i0 * input_size() * foo + i1 * foo + ir;
            std::size_t j = i1 * input_size() * foo + i0 * foo + ir;
            std::size_t k = i1 * input_size() * foo + ir * input_size() + i0;
            for (std::size_t l = 0; l < output_size(); ++l) {
              contract_assert(abs(data[rank][i * output_size() + l] - data[rank][j * output_size() + l]) < 1e-5);
              contract_assert(abs(data[rank][i * output_size() + l] - data[rank][k * output_size() + l]) < 1e-5);
            }
          }
  }

  autodiff_t& operator+=(const autodiff_t& arg) {
    contract_assert(shape() == arg.shape());
    for (std::size_t i = 0; i < diff_rank(); ++i)
      for (std::size_t j = 0; j < data[i].size(); ++j) data[i][j] += arg.data[i][j];
    return *this;
  }

  friend autodiff_t operator+(autodiff_t left, const autodiff_t& right) {
    left += right;
    return left;
  }

  autodiff_t& operator-=(const autodiff_t& arg) {
    contract_assert(shape() == arg.shape());
    for (std::size_t i = 0; i < diff_rank(); ++i)
      for (std::size_t j = 0; j < data[i].size(); ++j) data[i][j] -= arg.data[i][j];
    return *this;
  }

  friend autodiff_t operator-(autodiff_t left, const autodiff_t& right) {
    left -= right;
    return left;
  }

  autodiff_t& operator*=(double x) {
    for (auto& vec : data)
      for (auto& el : vec) el *= x;
    return *this;
  }

  friend autodiff_t operator*(autodiff_t arg, double x) {
    arg *= x;
    return arg;
  }

  friend autodiff_t operator*(double x, autodiff_t arg) {
    arg *= x;
    return arg;
  }

  autodiff_t& operator/=(double x) { return *this *= (1.0 / x); }

  friend autodiff_t operator/(autodiff_t arg, double x) {
    arg /= x;
    return arg;
  }
};

// f : A -> B
// Df : A -> L(A,B)
// Df(x) = ?
// Df(x)(d) = lim h->0 (f(x+hd)-f(x))/h
// so what is just Df(x) ?
//
// g : R -> B
// g(t) = f(x + dt)
// g(t) ~ g(0) + g'(0) t + g''(0) t^2 / 2 + ...
// f(x+td) ~ f(x) + Df(x)(d) t + DDF(x)(d)(d) t^2 / 2 + ...
// f(x+td) ~ f(x) + Df(x)(dt) + DDF(x)(dt)(dt) / 2 + ...
// ???
// kinda weird to get the `/ k!` bit
//
// D(f.g)(x)(d) = lim (f(x+dt).g(x+dt) - f(x).g(x)) / t
// = lim (f(x+dt).g(x+dt) - f(x+dt).g(x) + f(x+dt).g(x) - f(x).g(x)) / t
// = lim (f(x+dt).[g(x+dt) - g(x)] + [f(x+dt) - f(x)].g(x)) / t
// = f(x) . Dg(x)(d) + Df(x)(d) . g(x)
//
// D(x -> Df(x)(d))(d) = ?
// =? D(x -> Df(x))(d)(d)
// probably
//
// D2(f.g)(x)(d)(d) = ?
// = D(x -> f(x) . Dg(x)(d) + Df(x)(d) . g(x))(d)
// D(x -> Df(x)(d) . g(x))(d) = ?
// =? D2f(x)(d)(d) . g(x) + Df(x)(d) . Dg(x)(d)
//
// D2(f.g)(x)(d)(d) =?
// = D2f(x)(d)(d) . g(x) + 2 Df(x)(d) . Dg(x)(d) + f(x) . D2g(x)(d)(d)
// but this doesnt describe D2(f.g)(x) :'(
//
// D2f(x)(a)(b) =? D2f(x)(b)(a)
// maybe
//
// Dn(f.g)(x)(ds...) =? sum I,J partition {1..n} D_|I|(f)(x)(ds...[I]) . D_|J|(g)(x)(ds...[J])
// so how does Dn(f.g)(x) look ?
// is it really exponential ?
// methinks i dont understand multilinear maps well enough
// this is a weird sort of convolution of multilinear maps

double dot_specific(const autodiff_t& left, const autodiff_t& right, std::size_t rank, std::size_t i) {
  contract_assert(left.shape() == right.shape());
  contract_assert(left.diff_rank() > rank);
  contract_assert(left.data[rank].size() > i);
  const std::size_t N = left.input_size();
  const std::size_t M = left.output_size();
  const std::size_t DR = left.diff_rank();
  std::vector<std::size_t> carved(rank);
  ;
  double ret = 0.0;
  {
    std::size_t copy = i;
    for (std::size_t r = 0; r < rank; ++r, copy /= N) carved[r] = copy % N;
  }
  for (std::size_t left_mask = 0; left_mask < (1UZ << rank); ++left_mask) {
    std::size_t left_rank = std::popcount(left_mask);
    std::size_t right_rank = rank - left_rank;
    std::size_t left_i = 0;
    std::size_t right_i = 0;
    for (std::size_t r = 0; r < rank; ++r) {
      std::size_t& store = ((left_mask >> r) & 1) ? left_i : right_i;
      store = store * N + carved[r];
    }
    for (std::size_t m = 0; m < M; ++m)
      ret += left.data[left_rank][left_i * M + m] * right.data[right_rank][right_i * M + m];
  }
  return ret;
}

// f,g : R^a -> R^b
// f.g = f.g : R^a -> R
// D(f.g)(x) = Df(x)(g(x)) + Dg(x)((f(x))
// D(f.g)(x)(d) = ?
// D2(f.g)(x) = ?
autodiff_t dot(const autodiff_t& left, const autodiff_t& right) {
  contract_assert(left.shape() == right.shape());
  contract_assert(left.diff_rank() <= 64);
  const std::size_t N = left.input_size();
  const std::size_t M = left.output_size();
  const std::size_t DR = left.diff_rank();
  autodiff_t ret(N, 1, DR);
  std::vector<std::size_t> carved;
  carved.reserve(DR - 1);
  for (std::size_t rank = 0; rank < DR; ++rank) {
    if (rank) carved.emplace_back();
    for (std::size_t i = 0; i < ret.data[rank].size(); ++i) {
      ret.data[rank][i] = dot_specific(left, right, rank, i);
      // {
      //   std::size_t copy = i;
      //   for (std::size_t r = 0; r < rank; ++r, copy /= N) carved[r] = copy % N;
      // }
      // for (std::size_t left_mask = 0; left_mask < (1UZ << rank); ++left_mask) {
      //   std::size_t left_rank = std::popcount(left_mask);
      //   std::size_t right_rank = rank - left_rank;
      //   std::size_t left_i = 0;
      //   std::size_t right_i = 0;
      //   for (std::size_t r = 0; r < rank; ++r) {
      //     std::size_t& store = ((left_mask >> r) & 1) ? left_i : right_i;
      //     store = store * N + carved[r];
      //   }
      //   for (std::size_t m = 0; m < M; ++m)
      //     ret.data[rank][i] += left.data[left_rank][left_i * M + m] * right.data[right_rank][right_i * M + m];
      // }
    }
  }
  return ret;
}
