#include "pointwise"

#include <iostream>
#include <string>
#include <vector>

template <typename T> void dbg_add(T a, T b) {
  using namespace ivl::pointwise::arithmetic;
  using namespace ivl::pointwise::dbg; // operator<<;
  std::cout << "  a: " << a << std::endl;
  std::cout << "  b: " << b << std::endl;
  std::cout << "a+b: " << a + b << std::endl;
  std::cout << "a-b: " << a - b << std::endl;
  std::cout << std::endl;
}

int main() {

  dbg_add<std::vector<int>>({1, 2, 3}, {3, 2, 4});

  return 0;
}
