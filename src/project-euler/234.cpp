#include <iostream>
#include <ranges>
#include <vector>

#include <ivl/io/stlutils.hpp>
#include <ivl/logger>

const size_t TARGET = 999966663333;

std::vector<size_t> primes;

// <0, hi]
size_t divcnt(size_t hi, size_t m) {
  return hi / m;
}

// [lo, hi]
size_t divcnt(size_t lo, size_t hi, size_t m) {
  return divcnt(hi, m) - divcnt(lo - 1, m);
}

size_t divsum(size_t hi, size_t m) {
  auto tmp = hi / m;
  return (hi, m, tmp, m * tmp * (tmp + 1) / 2);
}

size_t divsum(size_t lo, size_t hi, size_t m) {
  return divsum(hi, m) - divsum(lo - 1, m);
}

int main() {
  {
    size_t            aprox_size = sqrt(TARGET) * 2 + 100;
    std::vector<bool> mem(aprox_size, false);
    for (size_t i = 2; i < aprox_size; ++i)
      if (not mem[i]) {
        primes.push_back(i);
        for (size_t j = i; j < aprox_size; j += i)
          mem[j] = true;
      }
    LOG(primes.size());
    LOG(ivl::io::Elems {primes | std::views::take(10)});
  }

  size_t sol = 0;

  // wish i had views::pairwise
  for (size_t i = 1; i < primes.size(); ++i) {
    auto a = primes[i - 1];
    auto b = primes[i];

    if (a * a > TARGET)
      break;

    size_t delta = 0;
    delta += (divsum(a * a + 1, std::min(TARGET, b * b - 1), a));
    delta += (divsum(a * a + 1, std::min(TARGET, b * b - 1), b));
    delta -= 2 * (divsum(a * a + 1, std::min(TARGET, b * b - 1), a * b));
    // LOG(a, b, delta);
    sol += delta;
  }

  LOG(sol);
}
