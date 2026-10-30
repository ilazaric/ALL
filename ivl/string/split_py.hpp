#pragma once

#include <ivl/meta>
#include <ivl/ranges/split_py>
#include <string>
#include <vector>

namespace ivl::string {
std::vector<std::string> split_py(std::string_view sv) {
  return views::split_py(sv) | std::views::transform(meta::construct<std::string>) | std::ranges::to<std::vector>();
}

std::vector<std::string_view> split_py_view(std::string_view sv) {
  return views::split_py(sv) | std::ranges::to<std::vector>();
}
} // namespace ivl::string
