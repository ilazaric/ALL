// #include <ivl/logger>
#include <cassert>
#include <concepts>
#include <iostream>

#define LOG(...)

/*

  P = 1/m

  P_disjoint(left, right) = P**ceil(left/2) * P**ceil(right/2)

  P_subsume(left, inner, right) = // WLOG left <= right
  = P**left * P_subsume(0, inner, right-left)

  P_subsume(inner, right) // WLOG right > 0
  x -> inner-1-x
  x -> inner+right-1-x

  inner+right dim at start
  inner//2 + (inner+right)//2 equations

  x -> inner-1-x -> inner+right-1-(inner-1-x) = x+right
  x <--> x % right
  in mod space both equations are
  x <--> inner-1-x
  number of orbits of that
  involution

  x = inner-1-x (mod right)
  2x = inner-1 (mod right)
  right is odd -> one solution
  right is even ->
    inner is even -> no solution
    inner is odd -> 2 solutions

  dimension:
  right is odd -> (right+1)/2
  right is even, inner is odd -> right/2+1
  right is even, inner is even -> right/2

  P_overlap(left, common, right)
  = common + ceil (left-common)/2 + ceil (right-common)/2 if common <= left & right

  (A [A A A) A]
  (A A [B B A A) B B]

  x --> left+common-1-x
  x --> left*2+right+common-1-x
  comp: left*2+right+common-1-(left+common-1-x) = left+right+x

  elements in left or right have just one equation,
  so nothing important can be concluded from them

  left, common, right
  ->
  left-1, common-1, right+1

  if common is smallest, then common + ceil (left-common)/2 + ceil (right-common)/2
  if left is smallest, then P_subsume(common-left, right+left)

  disjoint:
  sum_{1<=a,b & a+b<=n} P**ceil(a/2) * P**ceil(b/2) * (n-a+1-b+1) * (n-a+1-b+1 -1)
  = sum_x P**x * sum_{1<=a,b & a+b<=n & ceil a/2+ceil b/2=x} (n-a+1-b+1) * (n-a+1-b+1 -1)

  sum_{1<=2a,2b & 2a+2b<=n} P**a * P**b * (n-2a+1-2b+1) * (n-2a+1-2b+1 -1)
  sum_{1<=2a,2b+1 & 2a+2b+1<=n} P**a * P**(b+1) * (n-2a+1-2b-1+1) * (n-2a+1-2b-1+1 -1)
  ...

  sum_{1<=2a,2b & 2a+2b<=n} P**a * P**b * (n-2a+1-2b+1) * (n-2a+1-2b+1 -1)
  = sum_x P**x * sum_{1<=2a,2b & 2a+2b<=n & a+b=x} (n-2a+1-2b+1) * (n-2a+1-2b+1 -1)
  = sum_{2x<=n} P**x * sum_{1<=a,b & a+b=x} (n-2a+1-2b+1) * (n-2a+1-2b+1 -1)
  = sum_{2x<=n} P**x * sum_{1<=a<x} (n-2x+2) * (n-2x+1)
  = sum_{x<=n/2} P**x * x * (n-2x+2) * (n-2x+1) !!! 1/4 of disjoint

  S0(y) = sum_{x<y} P**x = (P**y - 1) / (P - 1)
  S1(y) = sum_{x<y} P**x * x
  sum_{x<y} S0(x) = sum P**x * (y-x-1) = sum (P**x - 1) / (P - 1) = (S0(y) - y) / (P - 1) = (P**y - 1 - y(P-1)) /
  (P-1)**2 = (y-1) S0(y) - S1(y) S1(y) = (y-1) S0(y) - (P**y - 1 - y(P-1)) / (P-1)**2 = ((y-1)(P-1)(P**y-1) - P**y +1
  -y(P-1)) / (P-1)**2 = (yPP**y - PP**y - yP**y - yP + P**y + P + y - 1 - P**y +1 -yP+y) / (P-1)**2 = (yPP**y - PP**y -
  yP**y - 2yP + P + 2y) / (P-1)**2 = (P**y (yP-P-y) - 2yP + P + 2y) / (P-1)**2

  (yPP-2yP-2P+y) - 2P+2 =? y (P-1)**2 = yPP - 2yP + y
  -2P - 2P+2 =? y (P-1)**2 =   0
  nope

  S0(y) = (P**y - 1) / (P - 1)
  S0(y+1) - S0(y) =? P**y
  (P**(y+1) -1) / (P-1) - (P**y -1) / (P-1) =? P**y
  (P**(y+1) -1) - (P**y -1) =? P**y * (P-1)
  PP**y -1 -P**y +1 =? P**y * (P-1)
  PP**y -P**y =? P**y * (P-1)
  good

  sum S0(x) = sum P**x (y-x) = sum (P**x -1) / (P-1) = (S0(y)-y) / (P-1)
  = y S0(y) - S1(y)
  S1(y) = y S0(y) - (S0(y)-y) / (P-1)

  S1(y+1) - S1(y) =? P**y * y
  (y+1) S0(y+1) - (S0(y+1)-y-1) / (P-1)
  - y S0(y) + (S0(y)-y) / (P-1)
  =? P**y * y
  (y+1)S0(y+1)(P-1) - (S0(y+1)-y-1)
  - y S0(y)(P-1) + (S0(y)-y)
  =? P**y * y (P-1)
  (y+1)S0(y+1)(P-1) - (P**y-1)- y S0(y)(P-1) =? P**y * y (P-1)
  S0(y+1)(P-1) + yS0(y+1)(P-1) - (P**y-1)- y S0(y)(P-1) =? P**y * y (P-1)
  S0(y+1)(P-1) + y P**y (P-1) - (P**y-1) =? P**y * y (P-1)
  (P**(y+1) - 1) + y P**y (P-1) - (P**y-1) =? P**y * y (P-1)
  P**(y+1) -1 + y P**y (P-1) - P**y +1 =? P**y * y (P-1)
  PP**y + y P**y (P-1) - P**y =? P**y * y (P-1)
  PP**y + yP P**y - y P**y - P**y =? P**y * y (P-1)
  P + yP - y - 1 =? y (P-1)
  nope

  sum S0(x) = sum P**x (y-1-x) = sum (P**x -1)/(P-1) = (S0(y)-y)/(P-1)
  = (y-1)S0(y) - S1(y)
  S1(y) = (y-1)S0(y) - (S0(y)-y)/(P-1)

  S1(y+1) - S1(y) =? yP**y

  +[yS0(y+1) - (S0(y+1)-y-1)/(P-1)]
  -[(y-1)S0(y) - (S0(y)-y)/(P-1)]
  =? yP**y

  +[yS0(y+1) - (P**y-1)/(P-1)]
  -[yS0(y) -S0(y)]
  =? yP**y

  +[yP**y - (P**y-1)/(P-1)]
  + S0(y)
  =? yP**y

  - (P**y-1)/(P-1) + S0(y) =? 0
  TRUE

  S1(y) = (y-1)S0(y) - (S0(y)-y)/(P-1)
  = (y-1)(P**y -1)/(P-1) - (P**y -1 -y(P-1))/(P-1)**2

  probably will need S2 as well

  sum S1(x) = sum P**x * x * (y-1-x) = (y-1)S1(y) - S2(y)
  S2(y) = (y-1)S1(y) - sum S1(x)

  Sn(y) = An(y) P**y + Bn(y)

  Sn+1(y) = (y-1)Sn(y) - sum Sn(y)

  Sn+1(y+1) - Sn+1(y) =? P**y * y**(n+1)
  [ySn(y+1) - sum_y+1 Sn(x)] - [(y-1)Sn(y) - sum_y Sn(x)] =? P**y * y**(n+1)
  [y P**y * y**n -  Sn(y)] + Sn(y) =? P**y * y**(n+1)
  [y P**y * y**n ]  =? P**y * y**(n+1)
  true

  Sn(y) = sum_{0<=x<=y} P**x * x
  S0(y) = P/(P-1) * P**y -1/(P-1)

  sum Sn(x) = sum P**x * x**n * (y+1-x) = (y+1)Sn(y) - Sn+1(y)
  Sn+1(y) = (y+1)Sn(y) - sum Sn(x)

*/

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

// struct polynomial {
//   std::vector<mint> coefs;

//   size_t size() const { return coefs.size(); }

//   // Some conventions treat deg(0) == -1
//   // Usually you want to special case it
//   int64_t degree() const { return size() - 1; }

//   mint& operator[](size_t i) { return coefs[i]; }
//   const mint& operator[](size_t i) const { return coefs[i]; }

//   void trim() {
//     while (!coefs.empty() && coefs.back() == 0) coefs.pop_back();
//   }

//   polynomial& operator+=(const polynomial& p) {
//     while (size() < p.size()) coefs.emplace_back();
//     for (size_t i = 0; i < p.size(); ++i) coefs[i] += p[i];
//     trim();
//     return *this;
//   }

//   friend polynomial operator+(const polynomial& a, const polynmial& b) {
//     auto res = a;
//     res += b;
//     return res;
//   }

//   polynomial& operator-=(const polynomial& p) {
//     while (size() < p.size()) coefs.emplace_back();
//     for (size_t i = 0; i < p.size(); ++i) coefs[i] -= p[i];
//     trim();
//     return *this;
//   }

//   friend polynomial operator-(const polynomial& a, const polynmial& b) {
//     auto res = a;
//     res -= b;
//     return res;
//   }

//   friend polynomial operator*(const polynomial& a, const polynomial& b) {
//     if (a.size() == 0 || b.size() == 0) return {};

//     polynomial r;
//     r.coefs.resize(a.size() + b.size() - 1, 0);

//     for (int i = 0; i < a.size(); ++i)
//       for (int j = 0; j < b.size(); ++j) r[i + j] += a[i] * b[j];

//     r.trim();
//     return r;
//   }

//   polynomial& operator*=(const polynomial& o) { return *this = *this * o; }
// };

/*
  n=3
  m=2

  abc

  1,1: 3 * 3 = 9
  1,2: 2 * 3 * 2/2 = 6
  1,3: 2 * 3 * 1/2 = 3

  2,2: 2/2 + 2/4 = 3/2
  2,3: 2*2/4 = 1
  3,3: 1/2

  total: 21

  n=4
  m=2

  2,4: 2 * (1/4 + 1/8 * 2) = 1
 */

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
              exp += inner + right - (right % 2 == 1 ? (right+1) / 2 : right / 2 + inner % 2);

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
  // LOG((subsume*4).value);
  
  return disjoint + subsume + overlap + subsume_sym;
}

void solve() {
  int n, m, p;
  std::cin >> n >> m >> p;

  mint::mod = p;

  auto P = mint(m).inverse();
  // assert(P.value != 1);
  // auto Pmi = (P - 1).inverse();

  // std::vector<polynomial> As, Bs;
  // As.push_back({P*Pmi});
  // Bs.push_back({-Pmi});

  // for (int n = 1; n < 10; ++n) {
  //   As.push_back(As.back() * {1, 1});
  //   Bs.push_back(Bs.back() * {1, 1});
  //   for (size_t i = 0; i < As[n-1].size(); ++i) {

  //   }
  // }

  // auto S0 = [&](int y) { return mul(pow(P, y) - 1, Pmi); };

  // sum_{1<=a,b & a+b<=n} P**ceil(a/2) * P**ceil(b/2) * (n-a+1-b+1) * (n-a+1-b+1 -1)
  mint disjoint;

  for (int sum = 2; sum <= n; ++sum) {
    auto coef1 = mint(n - sum + 1 + 1) * mint(n - sum + 1 + 1 - 1);
    auto coef2 = P ^ (sum / 2);

    if (sum % 2 == 1) {
      disjoint += coef1 * coef2 * (sum - 1);
    } else {
      disjoint += coef1 * coef2 * (sum / 2 - 1 + (sum - sum / 2) / P);
    }
  }

  // sum_{1<=inner,left+inner+right<=n} P_subsume(left,inner,right) * (n-left-inner-right+1)
  mint subsume;
  mint subsume_sym;

  // left == right
  // P_subsume(left,inner,left) = P**(left + ceil(inner/2))
  // sum_{1<=inner,2left+inner<=n} P**(left + ceil(inner/2)) * (n-2left-inner+1)
  for (int sum = 1; sum <= n; ++sum) {
    auto coef1 = P ^ (sum / 2);
    auto coef2 = mint(n - sum + 1);
    subsume_sym += coef1 * coef2 * (sum - (sum % 2 == 0));
  }

  // left < right (wlog)
  // P_subsume(left,inner,right) = P**left * P_subsume(0,inner,right-left)
  // P(inner,right)
  // right is odd -> (right+1)/2
  // right is even, inner is odd -> right/2+1
  // right is even, inner is even -> right/2
  // (left+inner+right+1)/2 - (inner)/2
  // sum 1<=inner, left<right, left+inner+right=S<=n P**((S+1)/2 - (inner)/2) * (n-S+1)
  for (int sum = 1; sum <= n; ++sum) {
    // TODO
    for (int inner = 1; inner < sum; ++inner) {
      auto rem = sum - inner;
      // left + inner + right - ((right-left) % 2 == 1 ? (right+left+1) / 2 : (right+left) / 2 + inner % 2);
      subsume += (P ^ (sum - (rem % 2 == 1 ? (rem+1)/2 : rem/2 + inner%2))) * (n - sum + 1) * (sum - inner + 1 - (rem % 2 == 0)) * 2;
      // subsume += (P ^ (sum - (sum + 1) / 2 + inner / 2)) * (n - sum + 1) * ((sum - inner + 1) / 2) * 2;
    }
  }

  // sum 1<=left,right,left+common+right<=n P_overlap(left,common,right) * (n-S+1)
  // P_overlap(left,2x,right) = P_overlap(left,0,right) = P**((left+1)/2 + (right+1)/2)
  // P_overlap(left,2x+1,right) = P_overlap(left,1,right) = P_overlap(left-1,0,right+1) = P**(left/2 + right/2 + 1)
  mint overlap;

  for (int lr = 2; lr <= n; ++lr) {
    // TODO
    for (int common = 1; lr + common <= n; ++common) {
          // if (common % 2 == 0) {
          //   overlap += P ^ (common + left / 2 + right / 2);
          // } else {
          //   overlap += P ^ (common + (left - 1) / 2 + (right - 1) / 2 + 1);
          // }
      if (common % 2 == 0) {
        if (lr % 2 == 1) {
          overlap += (P ^ (common + lr / 2)) * (n - lr - common + 1) * (lr - 1) * 2;
        } else {
          overlap += (P ^ (common + lr / 2)) * (n - lr - common + 1) * (lr / 2 - 1) * 2;
          overlap += (P ^ (common + lr / 2 - 1)) * (n - lr - common + 1) * (lr - lr / 2) * 2;
        }
      } else {
        if (lr % 2 == 1) {
          overlap += (P ^ (common + lr / 2)) * (n - lr - common + 1) * (lr - 1) * 2;
        } else {
          overlap += (P ^ (common + lr / 2 - 1)) * (n - lr - common + 1) * (lr / 2 - 1) * 2;
          overlap += (P ^ (common + lr / 2)) * (n - lr - common + 1) * (lr - lr / 2) * 2;
        }
      }
    }
  }

  // LOG(P.value);
  // LOG((P*P).value);
  // LOG((P*13).value);
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
