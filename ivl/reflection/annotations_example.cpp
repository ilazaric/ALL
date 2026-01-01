#include <ivl/linux/raw_syscalls>
#include <cassert>
#include <iostream>
#include <meta>
#include <print>
#include <vector>

// IVL add_compiler_flags("-freflection -static")
// IVL test_only()

inline int adder(int a, int b, int c) {
  return a + b; // whoops, missed c
}

[[= ivl::test]] inline void test_passes() { assert(adder(1, 2, 0) == 3); }

[[= ivl::test]] inline void test_broken() { assert(adder(3, 4, 5) == 12); }
