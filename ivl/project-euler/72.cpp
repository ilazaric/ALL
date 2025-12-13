
#include <ivl/logger>

constexpr size_t maxd = 1'000'000;
// constexpr size_t maxd = 8;

size_t maxprime[maxd + 1];

size_t pow(size_t x, size_t e) {
  size_t r = 1;
  while (e) {
    if (e & 1) r *= x;
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

int main() {
  for (size_t i = 2; i <= maxd; ++i)
    if (maxprime[i] == 0)
      for (size_t j = i; j <= maxd; j += i)
        maxprime[j] = i;

  // LOG(maxprime[8]);
  // LOG(phi(8));

  size_t out = 0;
  for (size_t n = 2; n <= maxd; ++n)
    out += phi(n);
  LOG(out);
}
