#pragma once

// Mark functions with [[=ivl::test]] to run them in testing mode.

namespace ivl {
struct test_t {};
inline constexpr test_t test;
} // namespace ivl

namespace {
[[= ivl::test]] void test_example() {}
} // namespace
