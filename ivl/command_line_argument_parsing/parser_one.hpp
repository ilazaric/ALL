#pragma once

#include "raw_arguments"
#include <print>
#include <string_view>

namespace ivl::cmdline_parsing {
// Utility to implement common exactly-one-string parsing.
// Example:
// // parses iff "foo" passed
// struct parser<foo> : parser_one {
//   bool parse_one(foo& /* arg */, std::string_view sv) const {
//     return sv == "foo";
//   }
// };
struct parser_one {
  bool parse(this const auto& self, auto& arg, raw_arguments& rest) {
    if (rest.empty()) {
      std::println("missing argument");
      return false;
    }
    auto x = rest[0];
    rest.remove_prefix(1);
    return self.parse_one(arg, x);
  }
};
} // namespace ivl::cmdline_parsing
