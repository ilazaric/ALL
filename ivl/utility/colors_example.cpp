#include <ivl/utility/colors>
#include <print>

int main() {
  for (int i = 0; i < 256; i += 5) {
    for (int j = 0; j < 256; j += 2) std::print("{} ", ivl::background(i, 255 - i, j));
    std::println("{}", ivl::reset());
  }
}
