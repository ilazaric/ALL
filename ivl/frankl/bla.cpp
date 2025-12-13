#include <algorithm>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <ranges>
#include <set>
#include <unordered_set>

#include "family.hpp"
#include "tester.hpp"

std::mt19937 gen(42);

constexpr std::size_t N       = 10;
constexpr std::size_t MAXSIZE = (1uz << N);

using Fam = Family<N>;

using Set = std::size_t;

// // using Fam = std::bitset<MAXSIZE>;
// struct Fam : std::bitset<MAXSIZE> {
//   using std::bitset<MAXSIZE>::operator<;
// };
// // using MultiFam = std::array<std::size_t, MAXSIZE>;

Fam reduce(const Fam&);

std::ostream& operator<<(std::ostream& out, const Fam& af) {
  auto f = reduce(af);
  for (std::size_t set = 1; set < MAXSIZE; ++set)
    if (f[set]) {
      for (std::size_t idx = 0; idx < N; ++idx)
        if (set & (1uz << idx)) out << (char)('a' + idx);
      out << " ";
    }
  return out;
}

Set random_set() {
  std::uniform_int_distribution<std::size_t> dist(0, MAXSIZE - 1);
  return dist(gen);
}

/*

  potential generalization to multisets:
  A, B E F, A <= B
  --> #A <= #B

  wlog no singletons

  let A be smallest non-empty element of F
  let F_!A be {B E F : A not subset of B}
  let F_A be {B E F : A subset of B}
  F = F_A + F_!A
  gamma : X -> X U A
  gamma maps F_!A to F_A
  gamma maps at most 2^#A - 1 elements to the same element

  if #A = 2 then solvable
  each set disjoint with A induces a set that adds 2 to count
  sum = #F
  one of the elements of A = {a,b} appears at least #F/2 times

  #A = 3

  0 1 1 1 3
  6 / 3 = 2
  2 / 5 < 1/2
  :(

  if sum_{AeF}{#A} / #F / #U >= 1/2 then true

 */

struct Generator {
  Fam           family{1};
  std::set<Fam> seen{Fam{1}};

  const Fam& get() {
    do {
      if (family.count() == MAXSIZE) {
        family = Fam{1};
      }

      Set set{};

      while (family.test(set))
        set = random_set();

      for (std::size_t idx = 0; idx < MAXSIZE; ++idx)
        if (family.test(idx)) family.set(idx | set);
    } while (seen.count(family));

    seen.insert(family);
    return family;
  }
};

bool timeout() {
  auto        curr  = std::chrono::high_resolution_clock::now();
  static auto start = curr;
  return (curr - start) > std::chrono::seconds(15);
}

bool test_frankl(const Fam& f) {
  std::array<std::size_t, N> counts{};
  for (std::size_t set = 0; set < MAXSIZE; ++set)
    if (f.test(set)) {
      for (std::size_t idx = 0; idx < N; ++idx)
        if (set & (1uz << idx)) ++counts[idx];
    }
  return std::ranges::max(counts) * 2 >= f.count();
}

// bool test_multiconj(const Fam& f){
//   MultiFam mf{};
//   for (std::size_t set = 0; set < MAXSIZE; ++set)
//     mf[set] = f[set];
//   std::array<std::size_t, N> order;
//   std::iota(order.begin(), order.end(), 0uz);
//   std::ranges::shuffle(order, gen);
//   for (auto idx : order){
//     for (std::size_t set = 0; set < MAXSIZE; ++set)
//       if (set & (1uz << idx)){
//         mf[set ^ (1uz << idx)] += mf[set];
//         mf[set] = 0;
//       }

//     auto mf2 = mf;
//     for (std::size_t set = 0; set < MAXSIZE; ++set){
//       std::size_t maxunder = 0;
//       for (std::size_t idx = 0; idx < N; ++idx)
//         if (set & (1uz << idx))
//           maxunder = std::max(maxunder, mf2[set ^ (1uz << idx)]);

//       if (mf2[set] && mf2[set] < maxunder)
//         return false;

//       if (mf2[set] == 0)
//         mf2[set] = maxunder;
//     }
//   }

//   return true;
// }

Fam reduce(const Fam& f) {
  std::array<std::size_t, N> masks;
  std::ranges::fill(masks, (1uz << N) - 1);
  for (std::size_t set = 0; set < MAXSIZE; ++set)
    if (f[set])
      for (std::size_t idx = 0; idx < N; ++idx)
        if (set & (1uz << idx)) masks[idx] &= set;

  std::size_t mask = 0;
  for (std::size_t idx = 0; idx < N; ++idx)
    if (std::countr_zero(masks[idx]) == idx) mask |= (1uz << idx);

  Fam out{};
  for (std::size_t set = 0; set < MAXSIZE; ++set)
    if (f[set]) out[set & mask] = true;

  return out;
}

bool test_esize(const Fam& af) {
  auto f = reduce(af);

  std::size_t sum   = 0;
  std::size_t count = 0;
  std::size_t mask  = 0;
  for (std::size_t set = 0; set < MAXSIZE; ++set)
    if (f[set]) ++count, sum += std::popcount(set), mask |= set;

  if (2 * sum >= count * std::popcount(mask)) return true;

  return false;
}

double eval(const Fam& af) {
  auto f = reduce(af);

  std::size_t sum   = 0;
  std::size_t count = 0;
  std::size_t mask  = 0;
  for (std::size_t set = 0; set < MAXSIZE; ++set)
    if (f[set]) ++count, sum += std::popcount(set), mask |= set;

  return (double)sum / count / std::popcount(mask);
}

int main() {
  // {
  //   Fam f{};
  //   f[0] = 1;
  //   f[1] = 1;
  //   f[7] = 1;
  //   assert(test_esize(f));
  // }

  {
    std::size_t repcount = 0;
    Generator   gen;
    Fam         bestf = gen.get();
    double      out   = eval(bestf);
    for (; not timeout(); ++repcount) {
      auto& cf = gen.get();
      auto  co = eval(cf);
      if (co < out) out = co, bestf = cf;
    }
    std::cout << "repcount: " << repcount << std::endl;
    std::cout << "low eval: " << out << std::endl;
    std::cout << "example: " << bestf << std::endl;
    return 0;
  }

  Generator   gen;
  Tester<Fam> tester;
  ATTACH_TEST(tester, test_frankl);
  // ATTACH_TEST(tester, test_multiconj);
  ATTACH_TEST(tester, test_esize);
  for (std::size_t repcount = 0; not timeout(); ++repcount)
    tester.test(gen.get());
}
