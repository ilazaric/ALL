#include <map>

#include <ivl/logger>

constexpr size_t maxn = 10'000'000;

size_t maxprime[maxn + 1];

size_t pow(size_t x, size_t e) {
  size_t r = 1;
  while (e) {
    if (e & 1)
      r *= x;
    x *= x;
    e >>= 1;
  }
  return r;
}

size_t phi(size_t n) {
  size_t out = 1;
  while (n != 1) {
    size_t p = maxprime[n];
    size_t e = 1;
    n /= p;
    while (n % p == 0)
      n /= p, ++e;
    out *= pow(p, e - 1) * (p - 1);
  }
  return out;
}

std::map<uint8_t, uint8_t> digits(size_t n) {
  std::map<uint8_t, uint8_t> out;
  while (n) {
    ++out[n % 10];
    n /= 10;
  }
  return out;
}

int main() {
  for (size_t i = 2; i <= maxn; ++i)
    if (maxprime[i] == 0)
      for (size_t j = i; j <= maxn; j += i)
        maxprime[j] = i;

  size_t besti = 1e9, bestphi = 1;
  for (size_t i = 2; i < maxn; ++i) {
    auto p = phi(i);
    if (digits(i) == digits(p))
      if (besti * p > i * bestphi)
        besti = i, bestphi = p;
  }
  LOG(besti);
}
