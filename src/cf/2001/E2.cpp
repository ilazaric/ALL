#include <cassert>
#include <deque>
#include <ivl/io/conversion>
#include <ivl/io/stlutils.hpp>
#include <ivl/logger>
#include <ivl/nt/rtmint.hpp>
#include <ivl/pollpushpop/pollpushpop.hpp>
#include <map>
#include <ranges>
#include <set>
// #include <boost/lambda2.hpp>

// using namespace boost::lambda2;

// auto cacheify = []<typename Callable>(Callable&& callable){
//   return [callable = std::forward<Callable>(callable)]<typename... Ts>(Ts... args){
//     using Ret = std::invoke_result_t<Callable, Ts...>;
//     static std::map<std::tuple<Ts...>, Ret> cache;
//     std::tuple key(args...);
//     if (cache.contains(key))
//       return cache[key];
//     return cache[key] = callable(args...);
//   };
//  };

using ivl::nt::RTMint;

using Heap = std::vector<int32_t>;

// true if deterministic
bool pop(Heap& data) {
  uint32_t idx = 1;
  while (2 * idx < data.size()) {
    if (data[idx * 2] == data[idx * 2 + 1])
      return false;
    idx = idx * 2 + (data[idx * 2] < data[idx * 2 + 1]);
  }
  int32_t last = -1;
  while (idx) {
    std::swap(last, data[idx]);
    idx /= 2;
  }
  return true;
}

bool test1(Heap data) {
  return pop(data);
}
bool test2(Heap data) {
  return pop(data) && pop(data);
}

Heap heapify(Heap data) {
  for (uint32_t idx = data.size() - 1; idx > 1; --idx)
    data[idx / 2] += data[idx];
  return data;
}

const std::vector<Heap>& heaps(uint32_t n, uint32_t k) {
  static std::map<uint32_t, std::map<uint32_t, std::vector<Heap>>> cache;
  auto&                                                            ret = cache[n][k];
  if (!ret.empty())
    return ret;
  struct Magic {
    Heap                       data;
    std::vector<uint32_t>      stack {1};
    uint32_t                   n;
    std::vector<Heap>&         heaps;
    ivl::pollpushpop::MaskType poll() const {
      if (stack.size() == n + 1)
        return 0;
      return ivl::pollpushpop::full_mask << stack.back();
    }
    void push(ivl::pollpushpop::IndexType idx) {
      stack.push_back(idx);
      ++data[idx];
      if (stack.size() == n + 1) {
        heaps.push_back(heapify(data));
      }
    }
    void pop(ivl::pollpushpop::IndexType) {
      --data[stack.back()];
      stack.pop_back();
    }
  };
  Magic magic {.data = Heap(1 << n, 0), .n {n}, .heaps {ret}};
  ivl::pollpushpop::iterate(ivl::pollpushpop::bounded(1 << n), magic);
  return ret;
}

uint32_t cnt1(uint32_t n, uint32_t k, auto&& pred) {
  return std::ranges::count_if(heaps(n, k) | std::views::filter(pred), &test1);
}
uint32_t cnt1(uint32_t n, uint32_t k) {
  return std::ranges::count_if(heaps(n, k), &test1);
}

uint32_t cnt2(uint32_t n, uint32_t k, auto&& pred) {
  return std::ranges::count_if(heaps(n, k) | std::views::filter(pred), &test2);
}
uint32_t cnt2(uint32_t n, uint32_t k) {
  return std::ranges::count_if(heaps(n, k), &test2);
}

uint32_t c(uint32_t a, uint32_t b) {
  uint32_t x = 1, y = 1;
  for (uint32_t i = 0; i < b; ++i) {
    x *= a - i;
    y *= i + 1;
  }
  return x / y;
}

uint32_t mc(uint32_t a, uint32_t b) {
  return c(a + b - 1, b);
}

uint32_t lemma(uint32_t n, uint32_t k) {
  uint32_t ret = 0;
  /*
  case #2 = g(n,g,>l) * multichoose(2^n-1,l)
  case #3 = g'(n,g,<l) * #1(n,l)
   */
  for (uint32_t ls = 0; ls * 2 < k; ++ls)
    for (uint32_t gt = ls + 1; ls + gt <= k; ++gt)
      ret +=
        cnt2(n - 1, gt, [&](const Heap& h) { return std::max(h[2], h[3]) > ls; }) *
          mc((1 << (n - 1)) - 1, ls) +
        cnt1(n - 1, gt, [&](const Heap& h) { return std::max(h[2], h[3]) < ls; }) * cnt1(n - 1, ls);
  return 2 * ret;
}

void check(uint32_t n, uint32_t k) {
  LOG(cnt2(n, k), lemma(n, k));
}

// RTMint brute(uint32_t n, uint32_t k, uint32_t p){
//   assert(n <= 5);
//   RTMint::p = p;
//   auto& h = heaps(n, k);
//   LOG(h.size());
//   return std::ranges::count_if(h, &test);
// }

/*

      A
     / \
    B > C
   /
  D

  case #1 D == C --> failure
  case #2 D > C --> B branch
  case #3 D < C --> C branch

  #2(n,k)

  #2(1,k) = 1

  #2(n+1,k) = sum_{r+g+l=k, g>l} ?

  g(n,k,cond) = kinda like #2, but the bigger number satisfies cond

  #2(n,k) = g(n,k,noop)

  g(n+1,k,cond) = 2 sum_{r+g+l=k, g>l, cond(g)} case #2 + case #3

  case #2 = g(n,g,>l) * multichoose(2^n-1,l)

  case #3 = g'(n,g,<l) * #1(n,l)

  g' is like g but for #1

*/

int main() {
  // LOG(brute(3, 3, 998244353));
  check(3, 3);

  return 0;
}
