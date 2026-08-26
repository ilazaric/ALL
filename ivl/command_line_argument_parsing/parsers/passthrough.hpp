#pragma once

#include "../passthrough"
#include "../parser_declaration"
#include "../raw_arguments"
#include <print>

namespace ivl::cmdline_parsing {
template<>
struct parser<passthrough> {
  bool parse(passthrough& arg, raw_arguments& rest) const {
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
