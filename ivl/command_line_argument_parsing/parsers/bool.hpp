#pragma once

#include "../parser_declaration"
#include "../parser_one"
#include "../raw_arguments"
#include <ivl/format>
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
struct parser<bool> : parser_one {
  bool parse_one(bool& arg, std::string_view sv) const {
    if (sv == "1" || sv == "true" || sv == "on" || sv == "yes" || sv == "enable") {
      arg = true;
      return true;
    }
    if (sv == "0" || sv == "false" || sv == "off" || sv == "no" || sv == "disable") {
      arg = false;
      return true;
    }
    ivl::fmt::println("failed to parse boolean, argument: {:?}", sv);
    return false;
  }
};
} // namespace ivl::cmdline_parsing
