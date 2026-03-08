#pragma once

#include <ivl/exception>
#include <ivl/meta>
#include <ivl/reflection/prettier_types>
#include <ivl/reflection/utility>
#include <ivl/utility/colors>
#include <ivl/utility>
#include <format>
#include <functional>
#include <meta>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>

// https://nullprogram.com/blog/2020/08/01/

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
using command_line_arguments = std::span<const char*>;

template<typename>
struct parser;

template<>
struct parser<bool> {
  void parse_one(bool& arg, std::string_view sv) const {
    if (sv == "1" || sv == "true") {
      arg = true;
      return;
    }
    if (sv == "0" || sv == "false") {
      arg = false;
      return;
    }
    panic("failed to parse boolean, argument: {:?}", sv);
  }

  void parse(bool& arg, std::optional<std::string_view> eq, command_line_arguments& rest) const {
    if (eq) return parse_one(arg, *eq);

    if (!rest.empty()) {
      std::string_view nxt = rest[0];
      if (!nxt.starts_with("--")) {
        rest = rest.subspan(1);
        return parse_one(arg, nxt);
      }
    }

    arg = true;
  }
};

struct parser_one {
  void parse(this auto&& self, auto& arg, std::optional<std::string_view> eq, command_line_arguments& rest) {
    if (eq) return self.parse_one(arg, *eq);
    rest.empty() && panic("missing argument");
    self.parse_one(arg, rest[0]);
    rest = rest.subspan(1);
  }
};

template<std::floating_point Fp>
struct parser<Fp> : parser_one {
  void parse_one(Fp& arg, std::string_view sv) const {
    auto ret = std::from_chars<Fp>(sv.data(), sv.data() + sv.size(), arg);
    ret&& ret.ptr == sv.data() + sv.size() || panic("failed to parse floating-point, argument: {:?}", sv);
  }
};

template<std::integral Ip>
struct parser<Ip> : parser_one {
  void parse_one(Ip& arg, std::string_view sv) const {
    auto ret = std::from_chars<Ip>(sv.data(), sv.data() + sv.size(), arg);
    ret&& ret.ptr == sv.data() + sv.size() || panic("failed to parse integer, argument: {:?}", sv);
  }
};

template<meta::same_as_one_of<std::string_view, std::string, std::filesystem::path> Str>
struct parser<Str> : parser_one {
  void parse_one(Str& arg, std::string_view sv) const { arg = sv; }
};

template<>
struct parser<const char*> : parser_one {
  void parse_one(const char*& arg, std::string_view sv) const { arg = sv.data(); }
};

struct description {
  std::string_view contents;
};

struct A {
  bool x;
  bool y;
  std::string_view z;
  const char* w;
  std::string t;
  int u;
};

struct B : A {
  A a;
  A b;
  void f(const char*);
};

template<typename T>
concept parseable =
  requires(T& a, std::optional<std::string_view> b, command_line_arguments c) { parser<T>{}.parse(a, b, c); };

// clang-format off
consteval bool is_parseable_type(std::meta::info type) {
  return extract<bool>(substitute(^^parseable, {type}));
}
// clang-format on

template<typename... Args>
consteval void err(std::format_string<Args...> fmt, Args&&... args) {
  __builtin_constexpr_diag(2, "command_line_argument_parsing", std::format(fmt, std::forward<Args>(args)...));
}

consteval bool is_argument_optional(std::meta::info type) {
  // TODO
  return is_same_type(type, ^^bool);
}

consteval bool validate_sanity(std::meta::info type) {
  bool ret = true;

  if (!is_type(type)) {
    err("ICE: expected type, got: {:?}", type);
    return false;
  }

  type = dealias(type);

  if (is_const_type(type) || is_volatile(type)) {
    err("type {:?} shouldn't be const or volatile", type);
    type = remove_cv(type);
    ret = false;
  }

  if (is_reference_type(type)) {
    err("type {:?} shouldn't be reference", type);
    type = remove_reference(type);
    ret = false;
  }

  if (!is_class_type(type)) {
    err("type {:?} is not a class", type);
    return false;
  }

  if (reflection::is_child_of(type, ^^std)) {
    err("type {:?} is a stdlib type", type);
    return false;
  }

  auto ctx = std::meta::access_context::unchecked();

  {
    auto bases = bases_of(type, ctx);
    if (!bases.empty()) err("type {:?} has bases: {::?}", type, bases);
  }

  {
    auto nsdms = nonstatic_data_members_of(type, ctx);
    std::vector<std::meta::info> nonpublic(
      std::from_range, std::views::filter(nsdms, std::not_fn(std::meta::is_public))
    );
    if (!nonpublic.empty()) err("type {:?} has non-public non-static data members: {::?}", nonpublic);
  }

  return ret;
}

template<typename T>
void print_help(std::string_view program_name, bool passthrough) {
  namespace term = ivl::terminal_graphical_rendition;
  auto section = term::colors::FG_LIGHTGREEN;
  auto option = term::colors::FG_CYAN;
  std::print("{}Usage: {} {}[--help]", section, program_name, option);
  template for (constexpr auto member : reflection::nsdms(^^T)) {
    if constexpr (is_argument_optional(type_of(member))) {
      std::print(" [--{} [{}]]", identifier_of(member), reflection::display_string_of(type_of(member)));
    } else {
      std::print(" [--{} {}]", identifier_of(member), reflection::display_string_of(type_of(member)));
    }
  }
  if (passthrough) std::print("[-- [passthrough-args]...]");
  std::println("{}", term::foreground_reset{});
  // TODO: descriptions
}

template<typename T>
void parse(T& state, command_line_arguments& args) {
  while (!args.empty()) {
    std::string_view current = args[0];
    // TODO: allow for parsing plain non-option value
    // ....: `gcc -c **src.cpp** -o src.o`
    if (!current.starts_with('-')) break;
    if (current == "--") break;
    args = args.subspan(1);
    current.starts_with("--") || panic("option should start with '--', got: {:?}", current);
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
        p.parse(state.[:member:], eq, args);
        found = true;
        break;
      }
    }

    found || panic("unrecognized option: {:?}", current);
  }
}
} // namespace ivl::cmdline_parsing
