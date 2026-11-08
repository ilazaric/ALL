#pragma once

#include <string>
#include <string_view>

namespace ivl::string {
// TODO: this is slow, either do better (like KMP), or switch to something better (abseil?)
std::string replace(std::string_view body, std::string_view from, std::string_view to) {
  contract_assert(!from.empty());
  std::string ret;
  while (!body.empty()) {
    if (body.starts_with(from)) {
      ret += to;
      body.remove_prefix(from.size());
    } else {
      ret += body[0];
      body.remove_prefix(1);
    }
  }
  return ret;
}
} // namespace ivl::string
