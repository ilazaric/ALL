#pragma once

#include <ivl/reflection/json>
#include <source_location>
#include <string_view>

namespace ivl::testing {
inline void contract_assert_json(
  const auto& actual, std::string_view expected, std::source_location loc = std::source_location::current()
) noexcept {
  auto actual_json = ivl::to_json(actual);
  auto expected_json = nlohmann::json::parse(expected);
  if (actual_json != expected_json) {
    std::println(stderr, "!!! ERROR: FAILED CHECK AT {}:{}", loc.file_name(), loc.line());
    std::println(stderr, "actual:\n{}", actual_json.dump(2));
    std::println(stderr, "expected:\n{}", expected_json.dump(2));
    std::println(stderr, "diff:\n{}", nlohmann::json::diff(actual_json, expected_json).dump(2));
  }
  contract_assert(actual_json == expected_json);
}
} // namespace ivl::testing
