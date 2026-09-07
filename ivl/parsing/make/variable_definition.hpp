#pragma once

#include <string>
#include <string_view>

namespace ivl::parsing::make {
struct variable_definition {
  std::string name;
  std::string contents;
  bool recursively_expanded;
  bool overriden; // either passed via cmdline (make A=1), or override A=1
};

// for associative containers
struct variable_name_compare {
  using is_transparent = void;
  static std::string_view name(const variable_definition& v) { return v.name; }
  static std::string_view name(const std::string& s) { return s; }
  static std::string_view name(std::string_view sv) { return sv; }
  static bool operator()(const auto& a, const auto& b) { return name(a) < name(b); }
};
} // namespace ivl::parsing::make
