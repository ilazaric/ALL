#include "autodiff"
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

int main() {
  {
    autodiff_t arg(1, 1, 3);
    arg.data[0][0] = 5;
    arg.data[1][0] = 1;
    auto ret = polynomial({3, 2, 1}, arg);
    for (std::size_t r = 0; r < ret.diff_rank(); ++r) {
      std::println("rank: {}", r);
      for (std::size_t i = 0; i < ret.data[r].size(); ++i) std::println("data[r][{}] = {}", i, ret.data[r][i]);
    }
  }
}
