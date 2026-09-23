#pragma once

#ifdef IVL_KIND_TEST
#include <ivl/format>
#include <ivl/json>
#include <ivl/reflection/json>
#include <source_location>
#include <string_view>

namespace ivl::testing {
inline void contract_assert_json(
  const auto& actual, std::string_view expected, std::source_location loc = std::source_location::current()
) noexcept {
  auto actual_json = ivl::to_json(actual);
  auto expected_json = boost::json::parse(expected);
  if (actual_json != expected_json) {
    ivl::fmt::println(stderr, "!!! ERROR: FAILED CHECK AT {}:{}", loc.file_name(), loc.line());
    ivl::fmt::println(stderr, "actual:\n{:2}", actual_json);
    ivl::fmt::println(stderr, "expected:\n{:2}", expected_json);
    // TODO: nlohmann had diff, switched to boost, need to implement diff
    // ivl::fmt::println(stderr, "diff:\n{:2}", json::diff(actual_json, expected_json));
  }
  contract_assert(actual_json == expected_json);
}
} // namespace ivl::testing
#endif // IVL_KIND_TEST
