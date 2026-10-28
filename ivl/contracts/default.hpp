#pragma once

#include <ivl/format>
#include <cstddef>
#include <string_view>

namespace ivl {
// On failure, prints context, `width` extra characters on both sides (`2*width+1` in total).
bool check_text_equal(std::string_view expected, std::string_view actual, size_t width = 100) {
  if (expected == actual) return true;
  fmt::println(stderr, "text mismatch");
  fmt::println(stderr, "expected size: {}", expected.size());
  fmt::println(stderr, "actual   size: {}", actual.size());
  size_t idx = 0;
  while (idx < expected.size() && idx < actual.size() && expected[idx] == actual[idx]) ++idx;
  size_t lo = idx < width ? 0 : idx - width;
  size_t hi = idx + width + 1;
  auto expected_snippet = expected.substr(lo, hi - lo);
  auto actual_snippet = actual.substr(lo, hi - lo);
  ivl::fmt::println(stderr, "{:?} !=\n{:?}", expected_snippet, actual_snippet);
  return false;
}

void assert_text_equal(std::string_view expected, std::string_view actual, size_t width = 100) {
  // This separation means no work is done in `ignore` semantic.
  contract_assert(check_text_equal(expected, actual, width));
}
} // namespace ivl
