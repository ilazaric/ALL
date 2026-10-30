#pragma once

#include <ivl/meta>
#include <ivl/ranges/split>
#include <string>
#include <vector>

namespace ivl::string {
std::vector<std::string> split(std::string_view sv, std::string_view by) {
  return views::split(sv, by) | std::views::transform(meta::construct<std::string>) | std::ranges::to<std::vector>();
}

std::vector<std::string_view> split_view(std::string_view sv, std::string_view by) {
  return views::split(sv, by) | std::ranges::to<std::vector>();
}
} // namespace ivl::string
