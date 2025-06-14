#include "compare.hpp"

#include <array>

using Arr = std::array<int, 12>;

void consume_less();
void consume_equal();
void consume_greater();

void fn(Arr a, Arr b) {
  switch (a <=> b) {
  case std::strong_ordering::less:
    consume_less();
    break;
  case std::strong_ordering::equal:
    consume_equal();
    break;
    /*case std::strong_ordering::greater: // commenting out gives warning, good
        consume_greater();
        break;*/
    /*case std::weak_ordering::greater: // uncommenting errors out, as we want
        return;*/
  }
}
