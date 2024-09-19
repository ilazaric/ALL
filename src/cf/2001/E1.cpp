#include <ivl/logger>
#include <ivl/io/conversion>
#include <ivl/io/stlutils.hpp>
#include <map>
#include <ranges>
#include <deque>
#include <set>
#include <cassert>

struct RTMint {
  inline static int64_t p{1};
  int64_t data;
  RTMint() : data(0){}
  RTMint(int64_t data) : data(data % p){
    if (this->data < 0) this->data += p;
  }
};

RTMint operator+(RTMint a, RTMint b){return {a.data + b.data};}
RTMint operator-(RTMint a, RTMint b){return {a.data - b.data};}
RTMint operator*(RTMint a, RTMint b){return {a.data * b.data};}
RTMint& operator+=(RTMint& a, RTMint b){return a = a + b;}
RTMint& operator-=(RTMint& a, RTMint b){return a = a - b;}
RTMint& operator*=(RTMint& a, RTMint b){return a = a * b;}

void one(){
  int64_t n{cin};
  int64_t k{cin};
  int64_t p{cin};
  RTMint::p = p;

  auto pow = [&](RTMint x, int64_t e){
    RTMint r = 1;
    while (e){
      if (e & 1) r *= x;
      x *= x;
      e >>= 1;
    }
    return r;
  };

  std::vector sols(n+1, std::vector<RTMint>(k+1, 0));
  for (int64_t ck = 0; ck <= k; ++ck) sols[0][ck] = 1;

  std::vector prefix_sums(n+1, std::vector<RTMint>(k+2, 0));
  for (int64_t ck = 0; ck <= k; ++ck) prefix_sums[0][ck+1] = prefix_sums[0][ck] + sols[0][ck];

  std::vector<RTMint> finvs(k+3);
  finvs[0] = 1;
  for (int64_t ck = 1; ck <= k+2; ++ck)
    finvs[ck] = finvs[ck-1] * pow(ck, p-2);

  RTMint pow2m1 = 0;
  for (int64_t curr_n = 1; curr_n <= n; ++curr_n){
    LOG(curr_n);
    pow2m1 = 2*pow2m1+1;
    for (int64_t curr_k = 1; curr_k <= k; ++curr_k){
      RTMint bla = 1;
      RTMint truc = pow2m1;
      for (int64_t less_k = 0; less_k*2 < curr_k; ++less_k) {
        sols[curr_n][curr_k] = sols[curr_n][curr_k] + RTMint{2} * (prefix_sums[curr_n - 1][curr_k-less_k+1] - prefix_sums[curr_n - 1][less_k+1]) * bla;
        bla *= truc + less_k;
        bla *= finvs[less_k + 1];
      }
    }
    for (int64_t ck = 0; ck <= k; ++ck) prefix_sums[curr_n][ck+1] = prefix_sums[curr_n][ck] + sols[curr_n][ck];
  }

  std::cout << sols[n-1][k].data << std::endl;
}

int main(){

  for (int64_t t{cin}; t--;){
    one();
  }

  return 0;
}
