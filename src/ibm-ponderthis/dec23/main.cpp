#include <iostream>
#include <cstdint>
#include <ranges>
#include <algorithm>
#include <thread>
#include <fstream>
#include <string>

#include <ivl/io/stlutils.hpp>

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

#include <ivl/literals/ints.hpp>
using namespace ivl::literals::ints_exact;

#include <ivl/timer/timer.hpp>

std::vector<std::uint64_t> sieve(std::uint64_t bound){
  std::vector<std::uint64_t> out;
  std::vector<bool> marked(bound, false);
  std::uint64_t idx = 2;
  for (; idx*idx < bound; ++idx)
    if (!marked[idx]){
      out.push_back(idx);
      for (auto rem = idx*idx; rem < bound; rem += idx)
        marked[rem] = true;
    }
  for (; idx < bound; ++idx)
    if (!marked[idx])
      out.push_back(idx);
  return out;
}

auto range2vec(auto&& r){
  return std::vector(r.begin(), r.end());
}

auto primes3k2 = range2vec(sieve(1e8) | std::views::filter([](auto x){return x % 3 == 2;}));

bool valid(std::uint64_t k){
  while (k % 3 == 0) k /= 3;
  if (k % 3 == 2) return false;
  for (std::uint64_t d = 2; d*d <= k; ++d)
    if (k % d == 0){
      std::uint32_t count = 0;
      while (k % d == 0) k /= d, ++count;
      if (d % 3 == 2 && count % 2 == 1)
        return false;
    }
  return true;
}

bool valid2(std::uint64_t k){
  while (k % 3 == 0) k /= 3;
  if (k % 3 == 2) return false;

  for (auto p : primes3k2){
    if (p*p > k)
      break;
    auto count = 0_u32;
    while (k % p == 0)
      k /= p, ++count;
    if (count % 2 == 1)
      return false;
  }
  return true;
}

std::uint64_t compute_f_slow(std::uint64_t m){
  std::uint64_t count = 0;
  for (auto k : std::views::iota(m*m+1, (m+1)*(m+1))){
    if (valid(k))
      ++count;
  }
  return count;
}

std::uint64_t compute_f_bit_faster(std::uint64_t m){
  std::uint64_t count = 0;
  for (auto k : std::views::iota(m*m+1, (m+1)*(m+1))){
    if (valid2(k))
      ++count;
  }
  return count;
}

std::uint32_t vp(std::uint64_t n, std::uint64_t p){
  auto count = 0_u32;
  while (true){
    auto div = n / p;
    if (div * p != n)
      break;
    ++count;
    n = div;
  }
  return count;
}

std::uint64_t reduce(std::uint64_t n, std::uint64_t p){
  while (n % p == 0)
    n /= p;
  return n;
}

std::uint64_t compute_f_more_faster(std::uint64_t m){
  const auto lo = m*m+1;
  const auto hi = (m+1)*(m+1);
  std::vector<char> dead(hi-lo, false);
  for (auto idx : std::views::iota(lo, hi))
    if (reduce(idx, 3) % 3 == 2)
      dead[idx-lo] = true;
  for (auto p : primes3k2 | std::views::take_while([m](auto x){return x <= m+1;})){
    auto p2 = p*p;
    auto start = (lo + p - 1) / p * p;
    auto start2 = (lo + p2 - 1) / p2 * p2;
    
    while (start < hi){
      if (start == start2){
        if (!dead[start-lo] && vp(start/p2, p) % 2 == 1)
          dead[start-lo] = true;
        start += p;
        start2 += p2;
      } else {
        dead[start-lo] = true;
        start += p;
      }
    }
    
    // for (auto k = start;
    //      k < hi;
    //      k += p)
    //   dead[k-lo] |= 2;
    // for (auto k = start2;
    //      k < hi;
    //      k += p2){
    //   dead[k-lo] ^= 2;
    //   if (dead[k-lo])
    //     continue;
    //   if (vp(k/p2, p) % 2 == 1)
    //     dead[k-lo] = true;
    // }
    // for (auto k = start;
    //      k < hi;
    //      k += p)
    //   if (dead[k-lo] & 2)
    //     dead[k-lo] = true;
    
    // for (auto k = start;
    //      k < hi;
    //      k += p){
    //   if (dead[k - lo])
    //     continue;
    //   if (vp(k/p, p) % 2 == 0)
    //     dead[k - lo] = true;
    // }
  }
  return hi-lo-std::ranges::fold_left(dead, 0_u64, std::plus{});
}

int main(){
  // LOG(sieve(1e4));

  // LOG(compute_f_slow(11));
  // LOG(compute_f_slow(42));

  // {
  //   SCOPE_TIMER;
  //   for (auto idx : std::views::iota(1, 80))
  //     LOG(100*idx, compute_f_slow(100*idx));
  // }

  // {
  //   SCOPE_TIMER;
  //   LOG(compute_f_bit_faster(200000));
  // }

  // assert(argc == 3);

  // {
  //   SCOPE_TIMER;
  // }

  {
    SCOPE_TIMER;
    
    constexpr auto lo = 4'000'000_u32;
    constexpr auto hi = 5'000'000_u32;
    constexpr auto jobcount = 4_u32;
    constexpr auto target = 1'000'000_u32;
    constexpr auto len = hi-lo;
    constexpr auto joblen = len / jobcount;
    
    std::vector<std::thread> jobs;
    for (auto idx : std::views::iota(0_u32, jobcount))
      jobs.emplace_back([idx](){
        auto count = 0_u32;
        auto perc = 0_u32;
        std::ofstream fout("data" + std::to_string(idx));
        for (auto m = lo + idx; m < hi; m += jobcount){
          if (compute_f_more_faster(m) == target){
            LOG(idx, m);
            fout << m << std::endl;
          }
          ++count;
          // LOG(idx, count, m);
          auto nperc = count * 10000 / joblen;
          if (nperc != perc){
            perc = nperc;
            LOG(idx, perc);
          }
        }
      });

    for (auto& job : jobs)
      job.join();

    return 0;
  }

  {SCOPE_TIMER; LOG(compute_f_more_faster(4e6));}
  {SCOPE_TIMER; LOG(compute_f_more_faster(4.5e6));}
  {SCOPE_TIMER; LOG(compute_f_more_faster(5e6));}
  {SCOPE_TIMER; LOG(compute_f_more_faster(7e6));}
  {SCOPE_TIMER; LOG(compute_f_more_faster(1e7));}
  {SCOPE_TIMER; LOG(compute_f_more_faster(4e7));}
  return 0;

  {
    SCOPE_TIMER;
    LOG(compute_f_more_faster(200000));
    // return 0;
  }

  {
    SCOPE_TIMER;
    auto prevhighloc = 15000;
    auto prevhighval = compute_f_bit_faster(prevhighloc);
    auto highdelta = 0_u64;
    for (auto idx : std::views::iota(prevhighloc+1, 15500)){
      auto currval = compute_f_more_faster(idx);
      if (currval >= prevhighval){
        prevhighval = currval;
        prevhighloc = idx;
      } else {
        highdelta = std::max(highdelta, prevhighval - currval);
      }
    }
    LOG(highdelta);
  }

  {
    SCOPE_TIMER;
    for (auto idx : std::views::iota(1, 40))
      LOG(1000*idx, compute_f_more_faster(1000*idx));
  }
  
}
