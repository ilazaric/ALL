#include "builder.hpp"

struct S {int x; float y; char z; int w = x;};

constexpr bool test(){
  auto generator = ivl::refl::builder<S>().z('a').x(1);
  S znj = generator.build();
  return znj.x == 1 && znj.z == 'a' && znj.w == znj.x;
}

static_assert(test());
