#include <ivl/io/conversion>
#include <ivl/io/stlutils.hpp>
#include <ivl/logger>
#include <map>
#include <ranges>

using Perm = std::vector<uint32_t>;

uint32_t eval(const Perm& perm) {
  Perm inv(perm.size());
  for (auto idx : std::views::iota(0u, perm.size()))
    inv[perm[idx]] = idx;

  uint32_t last_pos = 0;
  uint32_t sol      = 0;
  for (auto idx : std::views::iota(0u, perm.size())) {
    auto pos = inv[idx];
    if (pos < last_pos) ++sol;
    last_pos = pos;
  }
  return sol;
}

uint32_t eval2(const Perm& perm) {
  Perm p2(perm.size());
  for (auto idx : std::views::iota(0u, perm.size()))
    p2[idx] = perm.rbegin()[idx];
  return eval(p2);
}

void one() {
  uint32_t n{cin};

  if (n % 2 == 0) {
    std::cout << -1 << std::endl;
    return;
  }

  Perm     perm(n);
  uint32_t a = 0, b = n - 1;
  for (auto idx : std::views::iota(0u, perm.size())) {
    if (idx % 2 == 0) perm[idx] = a++;
    else perm[idx] = b--;
  }

  LOG(eval(perm));
  LOG(eval2(perm));
  for (auto& x : perm)
    ++x;
  std::cout << ivl::io::Elems{perm} << std::endl;
}

int main() {
  // LOG(eval({0, 1, 2}));
  // LOG(eval2({0, 1, 2}));
  // return 0;

  for (uint32_t t{cin}; t--;) {
    one();
  }

  return 0;
}
