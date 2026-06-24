#include "autodiff"
#include <cmath>
#include <print>

autodiff_t polynomial(const std::vector<double>& coefs, const autodiff_t& arg) {
  contract_assert(arg.output_size() == 1);
  contract_assert(arg.diff_rank() >= 1);
  autodiff_t ret(arg.shape());
  for (std::size_t i = coefs.size() - 1; i + 1; --i) {
    ret.data[0][0] += coefs[i];
    if (i) ret = dot(ret, arg);
  }
  return ret;
}

autodiff_t norm2(const autodiff_t& arg) { return dot(arg, arg); }

// f o g
// f' o g * g'
// f'' o g * g'^2 + f' o g * g''
// f''' o g * g'^3 + 3 f'' o g * g' * g'' + f' o g * g'''
// horrible

// t(as...) = sum t(as..., ai-1) * (1 + #(ai-1))

// (exp o g)'(x) = exp(g(x)) * g'(x)
// '' = exp(g(x)) * g'(x)^2 + exp(g(x)) * g''(x)
// TODO: not best, taylor up to 100
autodiff_t exp_bad(const autodiff_t& arg) {
  contract_assert(arg.output_size() == 1);
  autodiff_t ret(arg.shape());
  autodiff_t tmp(arg.shape());
  tmp.data[0][0] = 1;
  for (std::size_t i = 0; i < 10000; ++i) {
    ret += tmp;
    tmp /= i + 1.0;
    tmp = dot(tmp, arg);
  }
  for (auto& vec : tmp.data)
    for (auto el : vec) {
      0;
      contract_assert(abs(el) < 1e-5);
    }
  return ret;
}

autodiff_t exp_good(const autodiff_t& arg) {
  contract_assert(arg.output_size() == 1);
  autodiff_t ret(arg.shape());
  ret.data[0][0] = std::exp(arg.data[0][0]);
  for (std::size_t i = 1; i < arg.diff_rank(); ++i) {
    autodiff_t tmp(arg.shape());
    tmp.data[i] = arg.data[i];
    autodiff_t tmp2(arg.shape());
    tmp2.data[0][0] = 1.0;
    std::size_t cnt = (arg.diff_rank() - 1) / i + 1;
    for (std::size_t j = 0; j < cnt; ++j) {
      tmp2 = dot(tmp2, tmp);
      tmp2 /= (double)(cnt - j);
      tmp2.data[0][0] = 1.0;
    }
    ret = dot(ret, tmp2);
  }
  return ret;
}

autodiff_t exp_best(autodiff_t arg) {
  contract_assert(arg.output_size() == 1);
  autodiff_t ret(arg.shape());
  double final_coef = std::exp(arg.data[0][0]);
  arg.data[0][0] = 0.0;
  ret.data[0][0] = 1.0;
  for (std::size_t r = 1; r < arg.diff_rank(); ++r) {
    ret = dot(ret, arg);
    ret /= (double)(arg.diff_rank() - r);
    ret.data[0][0] = 1.0;
  }
  ret *= final_coef;
  return ret;
}

// 1/f
// -f'/f^2

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

// f(x) = x^a
// df(x) = a x^(a-1)
// d(fog)(x) = a g(x)^(a-1) * dg(x)

autodiff_t pow(autodiff_t arg, double e) {
  contract_assert(arg.output_size() == 1);
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

void show1(const autodiff_t& arg) {
  for (std::size_t r = 0; r < arg.diff_rank(); ++r) {
    std::println("rank: {}", r);
    for (std::size_t i = 0; i < arg.data[r].size(); ++i) std::println("data[r][{}] = {}", i, arg.data[r][i]);
  }
  std::println();
}

bool close(double a, double b) { return abs(a - b) < 1e-5; }

bool close(const autodiff_t& a, const autodiff_t& b) {
  contract_assert(a.shape() == b.shape());
  for (std::size_t r = 0; r < a.diff_rank(); ++r)
    for (std::size_t i = 0; i < a.data[r].size(); ++i)
      if (!close(a.data[r][i], b.data[r][i])) return false;
  return true;
}

int main() {
  {
    std::println("validating polynomial ...");
    autodiff_t arg(1, 1, 3);
    arg.data[0][0] = 5;
    arg.data[1][0] = 1;
    auto ret = polynomial({3, 2, 1}, arg);
    contract_assert(close(ret.data[0][0], 38.0));
    contract_assert(close(ret.data[1][0], 12.0));
    contract_assert(close(ret.data[2][0], 2.0));
  }

  autodiff_t arg(1, 1, 5);
  arg.data[0][0] = 1.5;
  arg.data[1][0] = 1;
  autodiff_t arg2 = polynomial({0.1, 0.2, 0.1, -0.12, 0.1}, arg);

  {
    std::println("validating exp_bad ...");
    contract_assert(close(polynomial({0, 0, 1}, exp_bad(arg2)), exp_bad(arg2 * 2.0)));
  }

  {
    std::println("validating exp_good ...");
    contract_assert(close(polynomial({0, 0, 1}, exp_good(arg2)), exp_good(arg2 * 2.0)));
  }

  {
    std::println("validating exp_best ...");
    contract_assert(close(polynomial({0, 0, 1}, exp_best(arg2)), exp_best(arg2 * 2.0)));
  }

  {
    std::println("comparing exp_bad and exp_good ...");
    contract_assert(close(exp_bad(arg2), exp_good(arg2)));
  }

  {
    std::println("comparing exp_bad and exp_best ...");
    contract_assert(close(exp_bad(arg2), exp_best(arg2)));
  }

  {
    std::println("validating inverse ...");
    autodiff_t one(arg2.shape());
    one.data[0][0] = 1.0;
    contract_assert(close(one, dot(arg2, inverse(arg2))));
  }

  {
    std::println("validating pow ...");
    contract_assert(close(pow(arg2, 3), polynomial({0, 0, 0, 1}, arg2)));
    contract_assert(close(polynomial({0, 0, 0, 1}, pow(arg2, 1.0 / 3.0)), arg2));
  }
}
