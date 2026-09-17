#include <ivl/stl/string>
#include <iostream>
#include <ivl/format>

// IVL add_compiler_flags("-fsanitize=undefined,address")

int main() {
  for (auto s : ivl::split_py_range(std::string("   hello    world  123   ")))
    ivl::fmt::println("[{}]", s);
}
