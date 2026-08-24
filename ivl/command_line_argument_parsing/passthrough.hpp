#pragma once

#include "parser_declaration"
#include "raw_arguments"
#include <optional>
#include <print>
#include <string_view>

namespace ivl::cmdline_parsing {
struct passthrough {
  std::span<const char*> data;
};

template<>
struct parser<passthrough> {
  bool parse(passthrough& arg, std::optional<std::string_view> eq, raw_arguments& rest) const {
    if (eq) {
      std::println("passthrough does not support `=value` argument style");
      return false;
    }
    arg.data = rest.rest;
    rest.remove_prefix(rest.size());
    return true;
  }
};
} // namespace ivl::cmdline_parsing
