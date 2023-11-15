#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <numeric>

#include <ivl/io/conversion.hpp>
using ivl::io::conversion::cin;

#include <ivl/io/stlutils.hpp>
using namespace ivl::io::vector_elems;

#include <ivl/literals/ints.hpp>
using namespace ivl::literals::ints_exact;

int main(){
  for (auto ti : std::views::iota(0_u32, std::uint32_t{cin})){
    std::uint32_t n{cin}, m{cin}, k{cin};
    std::vector<std::uint32_t> a(n), b(m);
    std::cin >> a >> b;

    if (k > 2)
      k = k % 2 + 2;

    for (auto ki : std::views::iota(0_u32, k)){
      auto ait = std::ranges::min_element(a);
      auto bit = std::ranges::max_element(b);
      if (*ait < *bit)
        std::swap(*ait, *bit);
      std::swap(a, b);
    }

    if (k % 2)
      std::swap(a, b);

    std::cout << std::accumulate(a.begin(), a.end(), 0_u64) << std::endl;
  }
}
