#include <memory>
#include <cassert>

#include "to_free_function_pointer.hpp"

struct S {
  int fn(int) const&& {return 42;}
};

int main(){
  const S s;
  assert(ivl::to_free_function_pointer_v<&S::fn>(std::move(s), 12) == 42);
}
