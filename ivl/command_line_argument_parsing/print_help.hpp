#pragma once

#include <ivl/reflection/prettier_types>
#include <ivl/utility/colors>
#include <meta>
#include <print>

namespace ivl::cmdline_parsing {
consteval bool is_argument_optional(std::meta::info type) {
  // TODO
  return is_same_type(type, ^^bool);
}

template<typename T>
inline void print_help(std::string_view program_name, bool passthrough) {
  namespace term = ivl::terminal_graphical_rendition;
  auto section = term::colors::FG_LIGHTGREEN;
  auto option = term::colors::FG_CYAN;
  std::print("{}Usage: {} {}[--help]", section, program_name, option);
  if constexpr (parseable<T>) {
    if constexpr (is_argument_optional(^^T)) {
      std::print(" [`{}`]", reflection::display_string_of(^^T));
    } else {
      std::print(" `{}`", reflection::display_string_of(^^T));
    }
  } else {
    template for (constexpr auto member : reflection::nsdms(^^T)) {
      if constexpr (is_argument_optional(type_of(member))) {
        std::print(" [--{} [`{}`]]", identifier_of(member), reflection::display_string_of(type_of(member)));
      } else {
        std::print(" [--{} `{}`]", identifier_of(member), reflection::display_string_of(type_of(member)));
      }
    }
  }
  // TODO: name of passthrough args should be customizable
  if (passthrough) std::print(" [--] args ...");
  std::println("{}", term::foreground_reset{});
  // TODO: descriptions
}
} // namespace ivl::cmdline_parsing
