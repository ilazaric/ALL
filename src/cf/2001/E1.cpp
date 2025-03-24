#include <ivl/logger>
#include <ivl/io/conversion>
#include <ivl/io/stlutils.hpp>
#include <ivl/nt/rtmint.hpp>
#include <map>
#include <ranges>
#include <deque>
#include <set>
#include <cassert>

using ivl::nt::RTMint;

void one(uint32_t n, uint32_t k, uint32_t p){
  RTMint::p = p;

  auto pow = [](RTMint a, uint32_t b){
    RTMint ret = 1;
    while (b){
      if (b & 1) ret *= a;
      a *= a;
      b >>= 1;
    }
    return ret;
  };

  std::vector<RTMint> finv(505);
  finv[0] = 1;
  for (uint32_t i = 1; i < finv.size(); ++i)
    finv[i] = finv[i-1] * pow(i, p-2);

  auto choose = [&](RTMint x, uint32_t y) -> RTMint {
    RTMint ret = 1;
    for (uint32_t i = 0; i < y; ++i)
      ret *= x-i;
    return ret * finv[y];
  };

  auto multichoose = [&](RTMint x, uint32_t y) -> RTMint {
    return choose(x-1+y, y);
  };

  std::vector sols(n+1, std::vector<RTMint>(k+1, 0));
  for (auto& x : sols[1]) x = 1;
  std::vector prefix_sums(n+1, std::vector<RTMint>(k+2, 0));
  auto gen_row = [&](uint32_t curr_n){
    prefix_sums[curr_n][0] = 0;
    for (uint32_t curr_k = 0; curr_k <= k; ++curr_k)
      prefix_sums[curr_n][curr_k+1] = prefix_sums[curr_n][curr_k] + sols[curr_n][curr_k];
  };
  gen_row(1);
  // [lo, hi>
  auto query_sum = [&](uint32_t curr_n, uint32_t lo, uint32_t hi){
    return prefix_sums[curr_n][hi] - prefix_sums[curr_n][lo];
  };
  
  for (uint32_t curr_n = 2; curr_n <= n; ++curr_n){
    for (uint32_t curr_k = 0; curr_k <= k; ++curr_k){
      auto bla = pow(2, curr_n-1)-1;
      RTMint truc = 2;
      for (uint32_t ls = 0; ls*2 < curr_k; ++ls){
        sols[curr_n][curr_k] += query_sum(curr_n-1, ls+1, curr_k-ls+1) * truc * finv[ls];
        truc *= bla+ls;
      }
    }
    gen_row(curr_n);
  }

  std::cout << sols[n][k].data << std::endl;
}

int main(){

  for (uint32_t t{cin}; t--;){
    uint32_t n{cin};
    uint32_t k{cin};
    uint32_t p{cin};
    one(n, k, p);
  }

  return 0;
}
