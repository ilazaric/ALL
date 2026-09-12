#pragma once

#include <ivl/reflection/utility>
#include "../annotations"
#include "../parser_declaration"
#include "../raw_arguments"
#include <meta>
#include <print>

namespace ivl::cmdline_parsing {
template<typename T>
  requires(!annotations_of_with_type(^^T, ^^class_basic_t).empty())
struct parser<T> {
  bool parse(T& state, raw_arguments& rest) const {
    while (!rest.empty()) {
      auto curr = rest[0];
      if (curr == "--") break;
      if (!curr.starts_with("-")) break;
      if (curr == "--help") return false;
      rest.remove_prefix(1);
      if (!curr.starts_with("--")) {
        std::println("option should start with \"--\", got: {:?}", curr);
        return false;
      }
      auto name = curr.substr(2);
      const char* eq = nullptr;
      if (auto loc = name.find('='); loc != std::string_view::npos) {
        eq = name.substr(loc + 1).data();
        name = name.substr(0, loc);
      }
      raw_arguments resteq(&eq, &eq + !!eq);
      bool found = false;
      template for (constexpr auto member : reflection::nsdms(^^T)) {
        if (identifier_of(member) != name) continue;
        found = true;
        // for booleans only allow `--foo=bar` and `--foo` , not `--foo bar`
        if constexpr (is_same_type(type_of(member), ^^bool)) {
          if (!eq) {
            state.[:member:] = true;
            break;
          }
        }
        parser<typename[:decay(type_of(member)):]> p;
        if (!p.parse(state.[:member:], eq ? resteq : rest)) {
          std::println("during parsing option: {:?}", curr);
          return false;
        }
        break;
      }
      if (!found) {
        std::println("unrecognized option: {:?}", curr);
        return false;
      }
      if (eq && !resteq.empty()) {
        std::println("cannot parse payload after `=`: {:?}", curr);
        return false;
      }
    }
    return true;
  }

  void print_help() const {
    std::print("[");
    template for (constexpr auto member : reflection::nsdms(^^T)) {
      constexpr auto node = identifier_of(member);
      constexpr auto type = display_string_of(type_of(member));
      constexpr auto msg = define_static_string(std::format(" --{}:`{}`", node.name(), type));
      std::print("{}", msg);
    }
    std::print(" ]");
  }
};
} // namespace ivl::cmdline_parsing
