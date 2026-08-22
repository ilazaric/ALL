#pragma once

#include <span>
#include <string_view>

namespace ivl::cmdline_parsing {
// TODO: contract assertions
struct raw_arguments {
  std::span<const char*> rest;

  raw_arguments(const char** begin, const char** end) : rest(begin, end) {}

  size_t size() const { return rest.size(); }
  bool empty() const { return rest.empty(); }

  std::string_view operator[](size_t i) const { return rest[i]; }

  void remove_prefix(size_t n) { rest = rest.subspan(n); }
};
} // namespace ivl::cmdline_parsing
