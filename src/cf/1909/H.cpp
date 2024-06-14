#include <ranges>
#include <vector>
#include <algorithm>
#include <functional>
#include <numeric>
#include <string>
#include <queue>
#include <map>
#include <random>
#include <span>
#include <cassert>
#include <compare>
#include <cstring>

#include <ivl/literals/ints.hpp>
using namespace ivl::literals::ints_exact;

#include <ivl/io/stlutils.hpp>

#include <ivl/io/conversion.hpp>
using ivl::io::conversion::cin;

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

#include <ivl/timer/timer.hpp>

auto dists(std::uint32_t n){
  using Perm = std::vector<std::uint32_t>;
  std::map<Perm, std::uint32_t> dist;
  std::queue<Perm> Q;

  Perm p(n);
  std::iota(p.begin(), p.end(), 0);
  dist[p] = 0;
  Q.push(p);

  while (!Q.empty()){
    p = Q.front();
    Q.pop();
    auto d = dist[p];

    for (auto l : std::views::iota(0_u32, p.size()))
      for (auto r : std::views::iota(l+1, p.size()))
        if (l % 2 != r % 2){
          for (auto i = l; i < r; i += 2)
            std::swap(p[i], p[i+1]);
          if (!dist.contains(p)){
            dist[p] = d+1;
            Q.push(p);
          }
          for (auto i = l; i < r; i += 2)
            std::swap(p[i], p[i+1]);
        }
  }

  return dist;
}

using Perm = std::vector<std::uint32_t>;

Perm random_permutation(std::uint32_t n, auto& gen){
  Perm p(n);
  std::iota(p.begin(), p.end(), 0);
  std::ranges::shuffle(p, gen);
  return p;
}

std::uint64_t norm(const Perm& p){
  std::uint64_t out = 0;
  for (auto i : std::views::iota(0_u32, p.size()))
    out += i < p[i] ? p[i]-i : i-p[i];
  return out;
}

struct Move {
  std::uint32_t l, r;

  struct EndSentinel {};

  Move operator*() const {return {l, l+1};}
  Move& operator++(){l += 2; return *this;}
  Move begin() const {return *this;}
  EndSentinel end() const {return {};}
  friend bool operator==(Move m, EndSentinel){return m.l > m.r;}
};

Perm& operator*=(Perm& p, Move m){
  for (auto [x, y] : m)
    std::swap(p[x], p[y]);
  return p;
}

Perm& operator*=(Perm& p, const std::vector<Move>& v){
  for (auto m : v)
    for (auto [x, y] : m)
      std::swap(p[x], p[y]);
  return p;
}

Perm operator*(Perm p, Move m){
  p *= m;
  return p;
}

Perm operator*(Perm p, const std::vector<Move>& v){
  p *= v;
  return p;
}

template<auto Eval>
Move greedy(const Perm& p){
  Move best_move;
  std::uint64_t best_norm = -1;
  for (auto r : std::views::iota(1_u32, p.size()))
    for (auto l : std::views::iota(0_u32, r))
           if (l % 2 != r % 2){
             Move current_move{l, r};
             std::uint64_t current_norm = Eval(p * current_move);
             if (best_norm > current_norm){
               best_move = current_move;
               best_norm = current_norm;
             }
           }
  return best_move;
}

std::uint32_t evaluate(Perm p, auto&& strat){
  std::uint32_t cnt = 0;
  while (norm(p) != 0){
    auto mv = strat(p);
    p *= mv;
    if constexpr (requires {mv.size();}){
      cnt += mv.size();
    } else {
      ++cnt;
    }
  }
  return cnt;
}

std::uint64_t norm2(const Perm& p){
  std::uint64_t out = 0;
  for (auto i : std::views::iota(0_u32, p.size()))
    out += ((std::int64_t)p[i] - (std::int64_t)i) * ((std::int64_t)p[i] - (std::int64_t)i);
  return out;
}

// Delta(p, i) says change in evaluation if i <-> i+1 swap occurred
template<auto Delta>
Move delta_greedy(const Perm& p){
  using ET = decltype(Delta(p, 0));
  std::array<ET, 2> accs{0, 0};
  std::array<std::pair<ET, std::uint32_t>, 2> max_seens{std::pair{0, -2}, std::pair{0, -1}};
  Move best_move{0, 1};
  ET best_eval = Delta(p, 0);
  for (auto i : std::views::iota(0_u32, p.size()-1)){
    auto& acc = accs[i % 2];
    auto& max_seen = max_seens[i % 2];

    acc += Delta(p, i);
    if (acc - max_seen.first < best_eval){
      best_move = {max_seen.second+2, i+1};
      best_eval = acc - max_seen.first;
    }

    if (acc > max_seen.first)
      max_seen = {acc, i};
  }

  return best_move;
}

std::int64_t cubed(std::int64_t a, std::int64_t b){
  return abs(a-b) * (a-b) * (a-b);
}

std::int64_t delta_norm3(const Perm& p, std::uint32_t i){
  return cubed(p[i],i+1) + cubed(p[i+1],i) - cubed(p[i],i) - cubed(p[i+1],i+1);
}

std::int64_t delta_norm2(const Perm& p, std::uint32_t i){
  return (std::int64_t)p[i+1] - (std::int64_t)p[i];
}

std::int64_t delta_norm(const Perm& p, std::uint32_t i){
  return (p[i] < i ? -1 : 1) + (p[i+1] > i+1 ? -1 : 1);
}

double delta_exp(const Perm& p, std::uint32_t i){
  return
    +exp2(abs((std::int64_t)p[i] - (std::int64_t)(i+1)))
    +exp2(abs((std::int64_t)p[i+1] - (std::int64_t)(i)))
    -exp2(abs((std::int64_t)p[i] - (std::int64_t)(i)))
    -exp2(abs((std::int64_t)p[i+1] - (std::int64_t)(i+1)));
}

std::uint32_t d(std::uint32_t a, std::uint32_t b){
  return a > b ? a-b : b-a;
}

std::vector<std::uint32_t> fingerprint(const Perm& p){
  std::vector<std::uint32_t> out(p.size());
  for (auto i : std::views::iota(0_u32, p.size()))
    out.rbegin()[d(p[i], i)]++;
  return out;
}

Move reduce_max_dists_simple(const Perm& p){
  auto bestf = fingerprint(p);
  Move bestm;
  for (auto r : std::views::iota(1_u32, p.size()))
    for (auto l : std::views::iota(0_u32, r))
      if (l % 2 != r % 2){
        Move currm = {l, r};
        auto currf = fingerprint(p * currm);
        if (currf < bestf){
          bestf = currf;
          bestm = currm;
        }
      }
  return bestm;
}

Move reduce_max_dists_simple_faster(const Perm& p){
  auto bestf = fingerprint(p);
  Move bestm;
  for (auto l : std::views::iota(0_u32, p.size())){
    auto currf = fingerprint(p);
    for (auto r : std::views::iota(l+1, p.size()))
      if (l % 2 != r % 2){
        currf.rbegin()[d(p[r], r)]--;
        currf.rbegin()[d(p[r-1], r-1)]--;
        currf.rbegin()[d(p[r], r-1)]++;
        currf.rbegin()[d(p[r-1], r)]++;
        
        if (currf < bestf){
          bestf = currf;
          bestm = Move{l, r};
        }
      }
  }
  return bestm;
}

template<typename K, typename V>
std::strong_ordering cmp(const std::map<K, V>& a,
                         const std::map<K, V>& b){
  auto ait = a.rbegin();
  auto bit = b.rbegin();
  while (ait != a.rend() && bit != b.rend()){
    if (ait->first > bit->first){
      auto cmpres = ait->second <=> V{};
      if (cmpres != 0) return cmpres;
      ++ait;
      continue;
    }

    if (ait->first < bit->first){
      auto cmpres = V{} <=> bit->second;
      if (cmpres != 0) return cmpres;
      ++bit;
      continue;
    }
    
    auto cmpres = ait->second <=> bit->second;
    if (cmpres != 0) return cmpres;
    ++ait;
    ++bit;
  }

  for (; ait != a.rend(); ++ait){
    auto cmpres = ait->second <=> V{};
    if (cmpres != 0) return cmpres;
  }

  for (; bit != b.rend(); ++bit){
    auto cmpres = V{} <=> bit->second;
    if (cmpres != 0) return cmpres;
  }

  return std::strong_ordering::equal;
}

template<typename T>
std::strong_ordering cmp2(auto&& r,
                          const T& a,
                          const T& b){
  using V = std::remove_cvref_t<decltype(a.begin()->second)>;
  
  for (auto&& el : r){
    auto ait = a.find(el);
    auto bit = b.find(el);
    if (ait == a.end() && bit == b.end())
      continue;

    if (bit == b.end()){
      auto cmpres = ait->second <=> V{};
      if (cmpres != 0) return cmpres;
      continue;
    }

    if (ait == a.end()){
      auto cmpres = V{} <=> bit->second;
      if (cmpres != 0) return cmpres;
      continue;
    }

    auto cmpres = ait->second <=> bit->second;
    if (cmpres != 0) return cmpres;
  }

  return std::strong_ordering::equal;
}

Move reduce_max_dists_simple_fasterer(const Perm& p){
  std::map<std::uint32_t, std::int32_t> bestf;
  Move bestm;

  for (std::uint32_t parity : {0, 1}){
    std::map<std::uint32_t, std::int32_t> suffixf;
    std::uint32_t l = parity;
    for (std::uint32_t r = parity+1; r < p.size(); r += 2){
      --suffixf[d(p[r], r)];
      --suffixf[d(p[r-1], r-1)];
      ++suffixf[d(p[r-1], r)];
      ++suffixf[d(p[r], r-1)];

      if (cmp(bestf, suffixf) > 0){
        bestf = suffixf;
        bestm = {l, r};
      }

      if (cmp(suffixf, {}) > 0){
        suffixf = {};
        l = r+1;
      }
    }
  }

  return bestm;
}

#ifndef PARAM1
#define PARAM1 17
#endif // PARAM1

#ifndef PARAM2
#define PARAM2 std::int16_t
#endif // PARAM2

#ifndef PARAM3
#define PARAM3 std::uint32_t
#endif // PARAM3

Move reduce_max_dists_simple_fast3er(const Perm& p){
  constexpr std::uint32_t N = PARAM1;
  using T = PARAM2;
  using V = std::array<T, N>;
  using K = PARAM3;
  std::map<K, V> bestf;
  Move bestm;

  assert(std::cmp_less(p.size(), std::numeric_limits<T>::max()));
  
  for (std::uint32_t parity : {0, 1}){
    std::map<K, V> suffixf;
    auto get = [&](std::uint32_t x) -> T& {return suffixf[x/N].rbegin()[x%N];};
    std::uint32_t l = parity;
    for (std::uint32_t r = parity+1; r < p.size(); r += 2){
      --get(d(p[r], r));
      --get(d(p[r-1], r-1));
      ++get(d(p[r-1], r));
      ++get(d(p[r], r-1));

      if (cmp(bestf, suffixf) > 0){
        bestf = suffixf;
        bestm = {l, r};
      }

      if (cmp(suffixf, {}) > 0){
        suffixf = {};
        l = r+1;
      }
    }
  }

  return bestm;
}

Move reduce_max_dists_simple_alt(const Perm& p){
  constexpr std::uint32_t N = PARAM1;
  using T = PARAM2;
  using V = std::array<T, N>;
  using K = PARAM3;
  std::unordered_map<K, V> bestf;
  Move bestm;

  std::uint32_t max_dist = 0;
  for (auto i : std::views::iota(0_u32, p.size()))
    max_dist = std::max(max_dist, d(i, p[i]));
  ++max_dist;

  assert(std::cmp_less(p.size(), std::numeric_limits<T>::max()));
  
  for (std::uint32_t parity : {0, 1}){
    std::unordered_map<K, V> suffixf;
    auto get = [&](std::uint32_t x) -> T& {x = max_dist-x; return suffixf[x/N][x%N];};
    std::uint32_t l = parity;
    for (std::uint32_t r = parity+1; r < p.size(); r += 2){
      --get(d(p[r], r));
      --get(d(p[r-1], r-1));
      ++get(d(p[r-1], r));
      ++get(d(p[r], r-1));

      if (cmp2(std::views::iota(0_u32, max_dist/N+1), bestf, suffixf) > 0){
        bestf = suffixf;
        bestm = {l, r};
      }

      if (cmp2(std::views::iota(0_u32, max_dist/N+1), suffixf, {}) > 0){
        suffixf = {};
        l = r+1;
      }
    }
  }

  return bestm;
}

Move reduce_max_dists_attempt(const Perm& p){
  // constexpr std::uint32_t N = 64;
  using T = PARAM2;
  using V = std::vector<T>;
  // using K = PARAM3;
  // std::unordered_map<K, V> bestf;

  std::uint32_t max_dist = 0;
  for (auto i : std::views::iota(0_u32, p.size()))
    max_dist = std::max(max_dist, d(i, p[i]));
  ++max_dist;

  V bestf(max_dist+1);
  V zeroes(max_dist+1);
  Move bestm;

  assert(std::cmp_less(p.size(), std::numeric_limits<T>::max()));
  
  for (std::uint32_t parity : {0, 1}){
    // std::map<K, V> suffixf;
    V suffixf(max_dist+1);
    // auto get = [&](std::uint32_t x) -> T& {return suffixf[x/N].rbegin()[x%N];};
    auto change = [&](std::uint32_t x, T v){
      x = max_dist - x;
      // if (x < N)
      suffixf[x] += v;
    };
    std::uint32_t l = parity;
    for (std::uint32_t r = parity+1; r < p.size(); r += 2){
      change(d(p[r], r), -1);
      change(d(p[r-1], r-1), -1);
      change(d(p[r-1], r), +1);
      change(d(p[r], r-1), +1);

      if (bestf > suffixf){
        bestf = suffixf;
        bestm = {l, r};
      }

      if (suffixf > zeroes){
        std::ranges::fill(suffixf, 0);
        l = r+1;
      }
    }
  }

  return bestm;
}

Move reduce_max_dists(const Perm& p){
  std::uint32_t max_dist = 0;
  for (auto i : std::views::iota(0_u32, p.size()-1))
    max_dist = std::max(max_dist, d(p[i], i));

  constexpr std::uint32_t N = 30;
  std::array<std::int32_t, N> best_count{};
  Move best_move;
  std::uint32_t best_total{};
  for (auto l : std::views::iota(0_u32, p.size())){
    std::array<std::int32_t, N> curr_count{};
    for (auto r : std::views::iota(l+1, p.size()))
      if (l % 2 != r % 2){
        for (auto i : {0, 1}){
          auto delta = max_dist - d(p[r-i], r-i);
          if (delta < N) curr_count[delta]--;
        }

        for (auto i : {0, 1}){
          auto delta = max_dist - d(p[r-1+i], r-i);
          if (delta == -1) goto broken_exit;
          if (delta < N) curr_count[delta]++;
        }

        if (curr_count < best_count){
          best_count = curr_count;
          best_move = Move{l, r};
          best_total = 0;
        }

        if (curr_count == best_count)
          ++best_total;
      }
  broken_exit:;
  }

  // {
  //   LOG(max_dist, ivl::io::Elems{best_count}, best_total);
  //   static int repcnt = 0;
  //   if (++repcnt == 20)
  //     exit(0);
  // }

  auto before = fingerprint(p);
  auto after = fingerprint(p * best_move);
  assert(before > after);

  for (auto i : std::views::iota(0_u32, std::min(N, max_dist+1)))
    if (-best_count[i] != before.rbegin()[max_dist-i] - after.rbegin()[max_dist-i]){
      LOG(i, -best_count[i], before.rbegin()[max_dist-i] - after.rbegin()[max_dist-i]);
      exit(0);
    }

  return best_move;
}

std::vector<Move> partition_half(const Perm& p){
  
}

std::vector<Move> partition_parity(const Perm& p){
  std::uint32_t l = 0, r = p.size();
  while (true){
    
    if (l == r) break;
  }
}

/*

  if all values with same parity were increasing
  then i know how to solve it in N ops
  that is (N choose N/2) permutations
  not small i guess
  not huge i suppose
  all is (N choose N/2) * N/2! * (N-N/2)!
  suppose for sake of simplicity N=2K
  (2K choose K) * K!^2
  how does (2K choose K) compare with K! ?
  https://en.wikipedia.org/wiki/Central_binomial_coefficient
  (2K choose K) < 4^K
  not good haha

  more generally, any half-half coloring
  can be separated in linear time
  which doesnt give enough information :(

  0011
  0101 -> 0011
  1001 -> 0101 -> 0011
  ...

  000111222
  001011222
  010101222

  let || denote a "concatenation" of permutations
  for P and Q permutations of not necessarily same length
  let P || Q denote a permutation of #P + #Q length
  (P || Q)(t) =
  P(t) if t <= #P
  #P+Q(t-#P) otherwise

  || is associative

  let P |^ n denote repetitive use of || to
  concatenate n copies of P

  conjecture:
  P = ((1 2) || (1)) |^ N
  takes at least N operations to sort
  #P = 3N
  if true this indicates looking at max dist
  is kinda dogshit

  a   b   c   d
  b-1 a+1 d-1 c+1
  what a weird transformation
  can be implemented with 2 splays,
  and snipping between them
  
 */

int main(){

  if (0){
    for (auto n : std::views::iota(0, 20)){
      auto dist = dists(n);
      LOG(n, std::ranges::max(dist | std::views::values));
    }
  }

  {
    std::mt19937 gen(101);
    for (auto ti : std::views::iota(1)){
      auto p = random_permutation(3000, gen);
      // LOG(norm2(p*Move{0,1})-norm2(p), delta_norm2(p,0));
      // LOG((std::int64_t)norm2(p*Move{2,3})-(std::int64_t)norm2(p), delta_norm2(p,2));
      // LOG(norm2(p*Move{0,3})-norm2(p), delta_norm2(p,0)+delta_norm2(p,2));
      // LOG(ivl::io::Elems{std::span(p.data(), 4)});
      // LOG(ivl::io::Elems{std::span((p*Move{0,3}).data(), 4)});
      // exit(0);
      // LOG(p.size(), evaluate(p, greedy<norm>));
      // LOG(p.size(), evaluate(p, greedy<norm2>));
      // LOG(p.size(), evaluate(p, delta_greedy<delta_norm2>));
      // LOG(p.size(), evaluate(p, delta_greedy<delta_norm3>));
      // LOG(p.size(), evaluate(p, delta_greedy<delta_exp>));
      // LOG(p.size(), evaluate(p, reduce_max_dists_simple));
      // LOG(p.size(), evaluate(p, reduce_max_dists));
      // LOG(p.size(), evaluate(p, reduce_max_dists_simple_faster));
      // LOG(p.size(), evaluate(p, reduce_max_dists_simple_fasterer));
      // {SCOPE_TIMER; LOG(p.size(), evaluate(p, reduce_max_dists_simple_fast3er));}
      {SCOPE_TIMER; LOG(p.size(), evaluate(p, reduce_max_dists_attempt));}
      // {SCOPE_TIMER; LOG(p.size(), evaluate(p, reduce_max_dists_simple_alt));}

      if (ti == 1) return 0;
      
      // LOG(p.size(), evaluate(p, delta_greedy<delta_norm>));
    }
  }
  
}
