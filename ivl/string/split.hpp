#pragma once

#include <ivl/ranges/split>
#include <ivl/utility/construct>
#include <string>
#include <vector>

namespace ivl::string {
std::vector<std::string> split(std::string_view sv, std::string_view by) {
  return views::split(sv, by) | std::views::transform(construct<std::string>) | std::ranges::to<std::vector>();
}

std::vector<std::string_view> split_view(std::string_view sv, std::string_view by) {
  return views::split(sv, by) | std::ranges::to<std::vector>();
}
} // namespace ivl::string
