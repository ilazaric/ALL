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
  for (std::size_t i = 0; i < 100; ++i) {
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

// autodiff_t pow(const autodiff_t& arg, double e) { contract_assert(arg.output_size() == 1); }

void show1(const autodiff_t& arg) {
  for (std::size_t r = 0; r < arg.diff_rank(); ++r) {
    std::println("rank: {}", r);
    for (std::size_t i = 0; i < arg.data[r].size(); ++i) std::println("data[r][{}] = {}", i, arg.data[r][i]);
  }
  std::println();
}

bool close(double a, double b) { return abs(a - b) < 1e-5; }

int main() {
  {
    std::println("validating polynomial ...");
    autodiff_t arg(1, 1, 3);
    arg.data[0][0] = 5;
    arg.data[1][0] = 1;
    auto ret = polynomial({3, 2, 1}, arg);
    show1(ret);
    contract_assert(close(ret.data[0][0], 38.0));
    contract_assert(close(ret.data[1][0], 12.0));
    contract_assert(close(ret.data[2][0], 2.0));
  }

  {
    std::println("validating exp_bad ...");
    auto f1 = [](const autodiff_t& arg) { return polynomial({0, 0, 1}, exp_bad(arg)); };
    auto f2 = [](const autodiff_t& arg) { return exp_bad(arg * 2.0); };
    autodiff_t arg(1, 1, 5);
    arg.data[0][0] = 3;
    arg.data[1][0] = 1;
    show1(f1(arg) - f2(arg));
  }

  {
    std::println("validating exp_good ...");
    auto f1 = [](const autodiff_t& arg) { return polynomial({0, 0, 1}, exp_good(arg)); };
    auto f2 = [](const autodiff_t& arg) { return exp_good(arg * 2.0); };
    autodiff_t arg(1, 1, 5);
    arg.data[0][0] = 3;
    arg.data[1][0] = 1;
    show1(f1(arg) - f2(arg));
  }

  {
    std::println("validating exp_best ...");
    auto f1 = [](const autodiff_t& arg) { return polynomial({0, 0, 1}, exp_best(arg)); };
    auto f2 = [](const autodiff_t& arg) { return exp_best(arg * 2.0); };
    autodiff_t arg(1, 1, 5);
    arg.data[0][0] = 3;
    arg.data[1][0] = 1;
    show1(f1(arg) - f2(arg));
  }

  {
    std::println("comparing exp_bad and exp_good ...");
    auto f1 = [](const autodiff_t& arg) { return polynomial({0, 0, 1}, exp_good(arg)); };
    auto f2 = [](const autodiff_t& arg) { return exp_good(arg * 2.0); };
    autodiff_t arg(1, 1, 5);
    arg.data[0][0] = 3;
    arg.data[1][0] = 1;
    autodiff_t arg2 = polynomial({2, 1, 3, 4, 5}, arg);
    show1(exp_bad(arg) - exp_good(arg));
  }

  {
    std::println("comparing exp_bad and exp_best ...");
    auto f1 = [](const autodiff_t& arg) { return polynomial({0, 0, 1}, exp_good(arg)); };
    auto f2 = [](const autodiff_t& arg) { return exp_good(arg * 2.0); };
    autodiff_t arg(1, 1, 5);
    arg.data[0][0] = 3;
    arg.data[1][0] = 1;
    autodiff_t arg2 = polynomial({2, 1, 3, 4, 5}, arg);
    show1(exp_bad(arg) - exp_best(arg));
  }

  {
    autodiff_t arg(1, 1, 5);
    arg.data[0][0] = 3;
    arg.data[1][0] = 1;
    auto ret = polynomial({7, 6, 5, 4, 3, 2, 1}, arg);
    show1(dot(ret, inverse(ret)));
  }
}
