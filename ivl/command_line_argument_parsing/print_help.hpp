#pragma once

#include <ivl/format>
#include <ivl/reflection/prettier_types>
#include <ivl/utility/colors>
#include "parser_declaration"
#include <meta>
#include <string_view>

namespace ivl::cmdline_parsing {
template<typename... Ts>
inline void print_help(std::string_view program_name) {
  namespace term = ivl::terminal_graphical_rendition;
  auto section = term::colors::FG_LIGHTGREEN;
  auto option = term::colors::FG_CYAN;
  ivl::fmt::print("{}Usage: {} {}[--help]", section, program_name, option);
  // TODO: gutted this while refactoring, need to improve
  template for (constexpr auto Ti : {^^Ts...}) {
    using P = parser<typename[:Ti:]>;
    if constexpr (requires { P{}.print_help(); }) {
      ivl::fmt::print(" ");
      P{}.print_help();
    } else {
      ivl::fmt::print(" `{}`", reflection::display_string_of(Ti));
    }
  }
  ivl::fmt::println("{}", term::foreground_reset{});
  // TODO: descriptions
}
} // namespace ivl::cmdline_parsing
