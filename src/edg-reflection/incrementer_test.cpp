#include "incrementer.hpp"

using namespace ivl::refl;

namespace test1 {
  using Inc = Incrementer<>;
  static_assert(Inc::get() == 0);
  static_assert((Inc::advance(), Inc::get() == 1));
  static_assert((Inc::advance(), Inc::get() == 2));
  static_assert((Inc::advance(), Inc::get() == 3));
  static_assert((Inc::advance(), Inc::get() == 4));
  static_assert((Inc::advance(), Inc::get() == 5));
  static_assert((Inc::advance(), Inc::get() == 6));
} // namespace test1

namespace test2 {
  constexpr Incrementer<> inc;
  static_assert(inc.get() == 0);
  static_assert((inc.advance(), inc.get() == 1));
  static_assert((inc.advance(), inc.get() == 2));
  static_assert((inc.advance(), inc.get() == 3));
  static_assert((inc.advance(), inc.get() == 4));
  static_assert((inc.advance(), inc.get() == 5));
  static_assert((inc.advance(), inc.get() == 6));
} // namespace test2
