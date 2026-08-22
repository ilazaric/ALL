#pragma once

#include "parser_declaration"
#include "raw_arguments"
#include <optional>
#include <print>
#include <string_view>

namespace ivl::cmdline_parsing {
/* TODO:
   from gdb manual:
   > Some options are described as accepting an argument which can be either on or off.
   > These are known as boolean options. Similarly to boolean settings commands—on and off
   > are the typical values, but any of 1, yes and enable can also be used as “true” value, and
   > any of 0, no and disable can also be used as “false” value. You can also omit a “true”
   > value, as it is implied by default.

   maybe add on,yes,enable as well */
template<>
struct parser<bool> {
  inline bool parse_one(bool& arg, std::string_view sv) const {
    if (sv == "1" || sv == "true") {
      arg = true;
      return true;
    }
    if (sv == "0" || sv == "false") {
      arg = false;
      return true;
    }
    // TODO: we should be able to remove this and add a generic top-level version?
    std::println("failed to parse boolean, argument: {:?}", sv);
    return false;
  }

  inline bool parse(bool& arg, std::optional<std::string_view> eq, raw_arguments& rest) const {
    if (eq) return parse_one(arg, *eq);

    // TODO: pretty sure this is stupid, change/remove, i think just --boolean[=value] is enough
    // ....: does that imply all optional args should only support `=value` parsing?
    if (!rest.empty()) {
      std::string_view nxt = rest[0];
      if (!nxt.starts_with("--")) {
        rest.remove_prefix(1);
        return parse_one(arg, nxt);
      }
    }

    arg = true;
    return true;
  }
};
} // namespace ivl::cmdline_parsing
