#pragma once

#include <ivl/reflection/prettier_types>
#include <ivl/utility/colors>
#include <meta>
#include <print>

namespace ivl::cmdline_parsing {
template<typename... Ts>
inline void print_help(std::string_view program_name) {
  namespace term = ivl::terminal_graphical_rendition;
  auto section = term::colors::FG_LIGHTGREEN;
  auto option = term::colors::FG_CYAN;
  std::print("{}Usage: {} {}[--help]", section, program_name, option);
  // TODO: gutted this while refactoring, need to improve
  template for (constexpr auto Ti : {^^Ts...}) { std::print(" `{}`", reflection::display_string_of(Ti)); }
  std::println("{}", term::foreground_reset{});
  // TODO: descriptions
}
} // namespace ivl::cmdline_parsing
