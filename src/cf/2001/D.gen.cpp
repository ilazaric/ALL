#include <cassert>
#include <deque>
#include <ivl/io/conversion>
#include <ivl/io/stlutils.hpp>
#include <ivl/logger>
#include <map>
#include <random>
#include <ranges>
#include <set>

int main() {
  std::random_device                 dev;
  std::mt19937                       rng(dev());
  int                                n = 30;
  std::uniform_int_distribution<int> dist(1, n);
  std::cout << n << std::endl;
  for (int i = 0; i < n; ++i)
    std::cout << dist(rng) << " ";
  std::cout << std::endl;
}
