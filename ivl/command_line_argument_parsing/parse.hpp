#pragma once

#include <ivl/meta>
#include <ivl/reflection/prettier_types>
#include <ivl/reflection/utility>
#include <ivl/utility/colors>
#include "parser_bool"
#include "parser_declaration"
#include "parser_floats"
#include "parser_ints"
#include "parser_one"
#include "parser_strings"
#include "print_help"
#include "raw_arguments"
#include <format>
#include <functional>
#include <meta>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <string_view>

// https://nullprogram.com/blog/2020/08/01/

// TODO:
// --long_option argument
// --long_option=argument
// --boolean_option // as-if =1 or =true
// -s argument // short option
// -abc // a,b,c are short boolean options
// no thanks on -abc
// do i even want short options

// --foo x
// --foo=x
// --foo= x // nope

namespace ivl::cmdline_parsing {
struct description {
  std::string_view contents;
};

// clang-format off
consteval bool is_parseable_type(std::meta::info type) {
  return extract<bool>(substitute(^^parseable, {type}));
}
// clang-format on

template<typename... Args>
consteval void err(std::format_string<Args...> fmt, Args&&... args) {
  // avoiding https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124404
  auto msg = std::format(fmt, std::forward<Args>(args)...);
  __builtin_constexpr_diag(2, "command_line_argument_parsing", std::string_view(msg));
}

template<typename T>
inline bool parse(T& state, raw_arguments& args) {
  if constexpr (parseable<T>) {
    if (!args.empty() && args[0] == std::string_view("--help")) return false;
    return parser<T>{}.parse(state, std::nullopt, args);
  } else {
    while (!args.empty()) {
      std::string_view current = args[0];
      // TODO: allow for parsing plain non-option value
      // ....: `gcc -c **src.cpp** -o src.o`
      if (!current.starts_with('-')) break;
      if (current == "--") break;
      if (current == "--help") return false;
      args.remove_prefix(1);
      if (!current.starts_with("--")) {
        std::println("option should start with '--', got: {:?}", current);
        return false;
      }
      auto name = current.substr(2);

      std::optional<std::string_view> eq;
      if (auto loc = name.find('='); loc != std::string_view::npos) {
        eq = name.substr(loc + 1);
        name = name.substr(0, loc);
      }

      bool found = false;
      template for (constexpr auto member : reflection::nsdms(^^T)) {
        if (identifier_of(member) == name) {
          parser<typename[:type_of(member):]> p;
          if (!p.parse(state.[:member:], eq, args)) {
            std::println("during parsing option: {:?}", current);
            return false;
          }
          found = true;
          break;
        }
      }

      if (!found) {
        std::println("unrecognized option: {:?}", current);
        return false;
      }
    }

    return true;
  }
}
} // namespace ivl::cmdline_parsing
