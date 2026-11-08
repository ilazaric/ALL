#pragma once

#include <ivl/utility/forward>
#include <string>
#include <string_view>

namespace ivl::string {
// TODO: constraint
std::string join(auto&& rg, std::string_view sep) {
  std::string ret;
  bool first = true;
  for (auto&& el : FWD(rg)) {
    // TODO
    if (!first) ret += sep;
    first = false;
    ret += el;
  }
  return ret;
}
} // namespace ivl::string
