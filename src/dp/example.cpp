#include <ivl/dp/weird.hpp>

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

struct Example : ivl::dp::Helper<Example, int(int, int)> {
  auto recursive_expression(int a, int b, auto tag) {
    if (a < b || b < 0)
      return 0;
    if (b == 0 || b == a)
      return 1;
    return compute(a - 1, b - 1, tag) + compute(a - 1, b, tag);
  }

} example;

int main() {
  LOG(example.compute(4, 2, ivl::dp::normal));
}
