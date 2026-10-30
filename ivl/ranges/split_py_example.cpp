#include <ivl/format>
#include "split_py"
#include <iostream>

// IVL add_compiler_flags("-fsanitize=undefined,address")

int main() {
  for (auto s : ivl::views::split_py(std::string("   hello    world  123   "))) ivl::fmt::println("[{}]", s);
}
