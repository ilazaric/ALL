#pragma once

// Mark functions with [[=ivl::test]] to run them in testing mode.
// Mark functions with [[=ivl::test_fail]] to test for failure (exit code nonzero).

// TODO: add utilities, rename file

namespace ivl {
struct test_t {};
inline constexpr test_t test;
// struct test_fail_t {};
// inline constexpr test_fail_t test_fail;
} // namespace ivl
