#include <ranges>
#include <vector>
#include <algorithm>
#include <functional>
#include <numeric>

#include <ivl/io/stlutils.hpp>

#include <ivl/io/conversion.hpp>
using ivl::io::conversion::cin;

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

int main(){
  for (auto ti : std::views::iota(0, (int)cin)){
    std::uint32_t n{cin};
    std::vector<std::uint32_t> l(n), r(n), c(n);
    std::cin >> ivl::io::Elems{l} >> ivl::io::Elems{r} >> ivl::io::Elems{c};
    std::ranges::sort(l);
    std::ranges::sort(r);
    std::ranges::sort(c);

    std::vector<std::uint64_t> lens;
    auto lit = l.begin();
    auto rit = r.begin();
    std::vector<std::uint32_t> stack;
    while (lit != l.end()){
      if (*lit <= *rit){
        stack.push_back(*lit);
        ++lit;
      } else {
        lens.push_back(*rit - stack.back());
        stack.pop_back();
        ++rit;
      }
    }

    while (rit != r.end()){
      lens.push_back(*rit - stack.back());
      stack.pop_back();
      ++rit;
    }

    std::ranges::sort(lens);
    std::ranges::reverse(lens);

    auto out = std::inner_product(c.begin(), c.end(), lens.begin(), 0ULL);
    std::cout << out << std::endl;
  }
}
