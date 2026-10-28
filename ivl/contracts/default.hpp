#pragma once

#include <ivl/format>
#include <cstddef>
#include <string_view>

namespace ivl {
void assert_text_equal(std::string_view expected, std::string_view actual, size_t width = 100) {
  if (expected == actual) return;
  fmt::println(stderr, "text mismatch");
  fmt::println(stderr, "expected size: {}", expected.size());
  fmt::println(stderr, "actual   size: {}", actual.size());
  size_t idx = 0;
  while (idx < expected.size() && idx < actual.size() && expected[idx] == actual[idx]) ++idx;
  size_t lo = idx < width ? 0 : idx - width;
  size_t hi_expected = std::min(idx + width, expected.size());
  size_t hi_actual = std::min(idx + width, actual.size());
  std::string_view expected_snippet(expected.data() + lo, expected.data() + hi_expected);
  std::string_view actual_snippet(actual.data() + lo, actual.data() + hi_actual);
  ivl::fmt::println(stderr, "{:?} !=\n{:?}", expected_snippet, actual_snippet);
  contract_assert(expected == actual);
}
} // namespace ivl
