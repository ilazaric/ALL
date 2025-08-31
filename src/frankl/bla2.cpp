#include <algorithm>
#include <bit>
#include <cassert>
#include <iostream>
#include <ranges>

#include "family.hpp"
#include "tester.hpp"

// constexpr std::size_t N = 10;
// constexpr std::size_t MAXSIZE = (1uz << N);

using Fam = Family<N>;

template <std::size_t N>
void dump() {
  auto v = generate<N>();
  std::cout << "counts: " << N << " -> " << v.size() << std::endl;
  // for (auto& f : v)
  //   std::cout << f << std::endl;
  if constexpr (N < 5) {
    dump<N + 1>();
  }
}

int main() { dump<0>(); }
