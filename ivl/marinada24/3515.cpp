#include <bit>
#include <iostream>

int main() {
  int sol = 0;
  for (uint32_t i = 1; i < (1 << 21); i += 2) {
    if (std::popcount(i) % 3 == 0) ++sol;
  }
  std::cout << sol << std::endl;
}
