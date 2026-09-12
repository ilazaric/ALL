#pragma once

#include "../parser_declaration"
#include "../raw_arguments"
#include <print>
#include <string_view>

namespace ivl::cmdline_parsing {
/* from gdb manual:
   > Some options are described as accepting an argument which can be either on or off.
   > These are known as boolean options. Similarly to boolean settings commands—on and off
   > are the typical values, but any of 1, yes and enable can also be used as “true” value, and
   > any of 0, no and disable can also be used as “false” value. You can also omit a “true”
   > value, as it is implied by default.

   added on,yes,enable as well */
template<>
struct parser<bool> {
  // TODO: maybe booleans should only be settable with `--foo=bar` syntax, think more
  inline bool parse(bool& arg, raw_arguments& rest) const {
    if (rest.empty()) return arg = true;
    std::string_view sv = rest[0];
    if (sv == "1" || sv == "true" || sv == "on" || sv == "yes" || sv == "enable") {
      rest.remove_prefix(1);
      arg = true;
      return true;
    }
    if (sv == "0" || sv == "false" || sv == "off" || sv == "no" || sv == "disable") {
      rest.remove_prefix(1);
      arg = false;
      return true;
    }
    return arg = true;
  }
};
} // namespace ivl::cmdline_parsing
