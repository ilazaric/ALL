// #include <ivl/logger>
#include <cassert>
#include <concepts>
#include <iostream>
#include <vector>

#define LOG(...)

struct mint {
  int value;
  // assumed to be prime
  inline static int mod = 0;

  auto operator<=>(const mint&) const noexcept = default;

  mint() : value(0) {}
  mint(std::integral auto x) : value(x % mod < 0 ? x % mod + mod : x % mod) {}

  static mint unsafe(int x) {
    mint r;
    r.value = x;
    return r;
  }

  friend mint operator+(mint a, mint b) {
    int x = a.value + b.value;
    return mint::unsafe(x < mod ? x : x - mod);
  }

  mint& operator+=(mint o) { return *this = *this + o; }

  friend mint operator-(mint a, mint b) {
    int x = a.value - b.value;
    return mint::unsafe(x < 0 ? x + mod : x);
  }

  mint& operator-=(mint o) { return *this = *this - o; }

  mint operator-() const { return mint::unsafe(value == 0 ? 0 : mod - value); }

  friend mint operator*(mint a, mint b) {
    long long x = a.value;
    x *= b.value;
    return mint::unsafe(x % mod);
  }

  mint& operator*=(mint o) { return *this = *this * o; }

  friend mint operator^(mint b, int e) {
    mint r = 1;
    while (e) {
      if (e % 2) r *= b;
      b *= b;
      e /= 2;
    }
    return r;
  }

  mint& operator^=(int o) { return *this = *this ^ o; }

  mint inverse() const { return *this ^ (mod - 2); }

  friend mint operator/(mint a, mint b) { return a * b.inverse(); }

  mint& operator/=(mint o) { return *this = *this / o; }
};

mint compute(int n, mint P) {
  mint disjoint;
  mint subsume;
  mint subsume_sym;
  mint overlap;

  for (int a = 0; a < n; ++a)
    for (int b = a; b < n; ++b)
      for (int c = 0; c < n; ++c)
        for (int d = c; d < n; ++d) {
          auto l1 = b - a + 1;
          auto l2 = d - c + 1;

          // if (std::min(l1, l2) != 2 or std::max(l1, l2) != 4) continue;
          // LOG(a, b, c, d);

          // disjoint
          if (a > d or c > b) {
            disjoint += P ^ (l1 / 2 + l2 / 2);
            continue;
          }

          // subsume
          if (a <= c and d <= b or c <= a and b <= d) {
            int left, inner, right;
            if (a <= c and d <= b) {
              left = c - a;
              inner = l2;
              right = b - d;
            } else {
              left = a - c;
              inner = l1;
              right = d - b;
            }

            if (left > right) std::swap(left, right);
            // LOG(left, inner, right);

            int exp = left;
            right -= left;
            left = 0;
            if (right == 0) {
              exp += inner / 2;
              subsume_sym += P ^ exp;
              continue;
            } else {
              // TODO
              exp += inner + right - (right % 2 == 1 ? (right + 1) / 2 : right / 2 + inner % 2);

              // left + inner + right - ((right-left) % 2 == 1 ? (right+left+1) / 2 : (right+left) / 2 + inner % 2);
            }

            // LOG(exp);

            subsume += P ^ exp;
            continue;
          }

          // overlap
          int left, common, right;
          if (a < c) {
            left = c - a;
            common = b - c + 1;
            right = d - b;
          } else {
            left = a - c;
            common = d - a + 1;
            right = b - d;
          }

          if (common % 2 == 0) {
            overlap += P ^ (common + left / 2 + right / 2);
          } else {
            overlap += P ^ (common + (left - 1) / 2 + (right - 1) / 2 + 1);
          }
        }

  LOG(disjoint.value, overlap.value, subsume.value, subsume_sym.value);

  return disjoint + subsume + overlap + subsume_sym;
}

void solve() {
  int n, m, p;
  std::cin >> n >> m >> p;

  mint::mod = p;

  mint P = mint(m).inverse();
  mint Pmi;
  if (P.value != 1) Pmi = 1/(P-1);
  mint P2mi;
  if ((P*P).value != 1) P2mi = 1/(P*P-1);

  // 1/(x-1)
  auto mi = [&](mint x) {
    return x == P ? Pmi : P2mi;
  };

  std::vector<mint> pows1{1};
  for (int i = 0; i < n + 5; ++i) pows1.push_back(pows1.back() * P);
  std::vector<mint> pows2{1};
  for (int i = 0; i < n + 5; ++i) pows2.push_back(pows2.back() * P * P);

  auto pow = [&](mint x, int e) {
    return (x == P ? pows1 : pows2)[e];
  };

  mint disjoint;

  {
    mint pow = 1;
    for (int sum = 2; sum <= n; ++sum) {
      auto coef1 = mint(n - sum + 1 + 1) * mint(n - sum + 1 + 1 - 1);
      auto coef2 = pow;

      if (sum % 2 == 1) {
        disjoint += coef1 * coef2 * (sum - 1);
      } else {
        disjoint += coef1 * coef2 * (P * (sum / 2 - 1) + (sum - sum / 2));
      }

      if (sum % 2 == 0) pow *= P;
    }
  }

  auto S0 = [&](int hi, mint P, mint = {}) -> mint {
    if (hi < 0) return 0;
    if (P.value == 1) return hi + 1;
    return (pow(P, (hi + 1)) - 1) * mi(P);
  };
 
  auto S1 = [&](int hi, mint P, mint = {}) -> mint {
    if (hi < 0) return 0;
    if (P.value == 1) return hi % 2 == 0 ? mint(hi/2) * (hi + 1) : mint(hi) * ((hi+1)/2);
    // TODO
    // sum Sn(x) = sum P^x x^n (y+1-x) = (y+1) Sn(y) - Sn+1(y)
    // Sn+1(y) = (y+1)Sn(y) - sum Sn(x)
    auto s0 = S0(hi, P);
    mint ret = (hi + 1) * s0;
    ret -= P * mi(P) * s0;
    ret += mi(P) * (hi + 1);
    return ret;
  };

// #define pow(a, b) ((a) ^ (b))

  mint subsume;
  mint subsume_sym;

  for (int sum = 1; sum <= n; ++sum) {
    auto coef1 = pow(P, (sum / 2));
    auto coef2 = mint(n - sum + 1);
    subsume_sym += coef1 * coef2 * (sum - (sum % 2 == 0));
  }

  for (int rem = 1; rem <= n; rem += 2) {
    auto coef1 = pow(P, (rem + 1 - (rem + 1) / 2)) * (rem + 1) * 2;
    subsume += coef1 * (S0(n-rem-1, P) * (n-rem) - S1(n-rem-1, P));
  }
 
  for (int rem = 2; rem <= n; rem += 2) {
    if (n - rem - 1 < 0) continue;
    auto coef1 = pow(P, (rem - (rem / 2))) * rem * 2;
    subsume += coef1 * (S0((n-rem-1)/2, P*P) * (n-rem) - S1((n-rem-1)/2, P*P)*2);
  }
 
  for (int rem = 2; rem <= n; rem += 2) {
    if (n - rem - 2 < 0) continue;
    auto coef1 = pow(P, (rem + 2 - (rem / 2))) * rem * 2;
    subsume += coef1 * (S0((n-rem-2)/2, P*P) * (n-rem-1) - S1((n-rem-2)/2, P*P) * 2);
  }

  // for (int rem = 1; rem <= n; rem += 2) {
  //   auto coef1 = pow(P, (rem + 1 - (rem + 1) / 2)) * (rem + 1) * 2;
  //   auto PP = pow(P, n - rem - 1);
  //   subsume += coef1 * (S0(n - rem - 1, P, PP) * (n - rem) - S1(n - rem - 1, P, PP));
  // }

  // for (int rem = 2; rem <= n; rem += 2) {
  //   if (n - rem - 1 < 0) continue;
  //   auto coef1 = pow(P, (rem - (rem / 2))) * rem * 2;
  //   auto PP = pow(P*P, (n - rem - 1) / 2);
  //   subsume += coef1 * (S0((n - rem - 1) / 2, P * P, PP) * (n - rem) - S1((n - rem - 1) / 2, P * P, PP) * 2);
  // }

  // for (int rem = 2; rem <= n; rem += 2) {
  //   if (n - rem - 2 < 0) continue;
  //   auto coef1 = pow(P, (rem + 2 - (rem / 2))) * rem * 2;
  //   auto PP = pow(P*P, (n - rem - 2) / 2);
  //   subsume += coef1 * (S0((n - rem - 2) / 2, P * P, PP) * (n - rem - 1) - S1((n - rem - 2) / 2, P * P, PP) * 2);
  // }

  mint overlap;

  for (int lr = 3; lr <= n; lr += 2) {
    auto coef1 = pow(P, (lr / 2)) * (lr - 1) * 2;
    auto PP = pow(P, n-lr);
    overlap += coef1 * (n - lr + 1) * (S0(n - lr, P, PP) - 1);
    overlap -= coef1 * S1(n - lr, P, PP);
  }

  for (int lr = 2; lr < n; lr += 2) {
    auto PP = pow(P*P, (n - 1 - lr) / 2);
    auto coef1 = S0((n - 1 - lr) / 2, P * P, PP) * (n - lr) - S1((n - 1 - lr) / 2, P * P, PP) * 2;
    coef1 *= pow(P, (lr / 2));
    overlap += coef1 * ((lr - 2) + lr * P);
  }

  for (int lr = 2; lr <= n; lr += 2) {
    if (n - lr - 2 < 0) continue;
    auto PP = pow(P*P, (n - lr - 2) / 2);
    auto coef1 = S0((n - lr - 2) / 2, P * P, PP) * (n - lr - 1) - S1((n - lr - 2) / 2, P * P, PP) * 2;
    coef1 *= pow(P, (lr / 2 + 1));
    overlap += coef1 * (lr - 2) * P;
    overlap += coef1 * lr;
  }

  LOG(disjoint.value, overlap.value, subsume.value, subsume_sym.value);

  auto res = disjoint + overlap + subsume + subsume_sym;

  // auto correct = compute(n, P);
  // assert(res == correct);

  std::cout << res.value << std::endl;
}

int main() {
  int t;
  std::cin >> t;
  for (int i = 0; i < t; ++i) solve();
}
