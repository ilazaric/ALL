#include <complex>
#include <ivl/io/stlutils.hpp>
#include <ivl/logger>
#include <vector>

using namespace std::literals::complex_literals;

using Real  = double;
using Poly  = std::vector<Real>;
using CPoly = std::vector<std::complex<Real>>;

template <typename T>
std::vector<T> mul(const std::vector<T>& a, const std::vector<T>& b) {
  std::vector<T> out(a.size() + b.size() - 1);
  for (uint32_t i = 0; i < a.size(); ++i)
    for (uint32_t j = 0; j < b.size(); ++j)
      out[i + j] += a[i] * b[j];
  return out;
}

CPoly euler_basic {1, 1i};

CPoly euler(uint32_t n) {
  CPoly out {1};
  for (uint32_t i = 0; i < n; ++i)
    out = mul(out, euler_basic);
  return out;
}

std::pair<Poly, Poly> split(uint32_t n) {
  const auto cis = euler(n);
  Poly       c(n + 1);
  Poly       s(n + 1);
  for (uint32_t i = 0; i <= n; ++i) {
    c[i] = cis[i].real();
    s[i] = cis[i].imag();
  }
  return {c, s};
}

int main() {
  for (uint32_t n = 1; n < 10; n += 2) {
    auto [c, s] = split(n);
    LOG(n, c, s);
  }
  LOG(split(2023).second.back());
}
