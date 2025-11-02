#include <ivl/stl/string>
#include <iostream>
#include <print>

// IVL add_compiler_flags("-fsanitize=undefined,address")

int main() {
  for (auto s : ivl::split_py_range(std::string("   hello    world  123   ")))
    std::println("[{}]", s);
}
