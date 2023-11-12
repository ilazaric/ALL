#include "logger.hpp"

using namespace ivl::logger::default_logger;

int f(int x, int y) { return x + y; }

int main() {

  LOG(1, 23, "a,,b", ',', f(1, 2));
  for (int i = 0; i < 5; ++i)
    LOG(i);

  return 0;
}
