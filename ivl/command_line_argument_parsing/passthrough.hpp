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
    if (!rest.empty()) {
      if (rest[0] == "--") rest.remove_prefix(1);
      else if (rest[0].starts_with("--")) {
        std::println("first argument to passthrough is option-like {:?}, add explicit \"--\" if not mistake", rest[0]);
        return false;
      }
    }
    arg.data = rest.rest;
    rest.remove_prefix(rest.size());
    return true;
  }
};
} // namespace ivl::cmdline_parsing
