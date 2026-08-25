#pragma once

#include <ivl/reflection/utility>
#include "annotations"
#include "parser_declaration"
#include "raw_arguments"
#include <meta>

namespace ivl::cmdline_parsing {
template<typename T>
  requires(!annotations_of_with_type(^^T, ^^class_basic_t).empty())
struct parser<T> {
  bool parse(T& state, std::optional<std::string_view> eq_, raw_arguments& rest) const {
    if (eq_) {
      std::println("class parsing does not support `=` style");
      return false;
    }
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
      std::optional<std::string_view> eq;
      if (auto loc = name.find('='); loc != std::string_view::npos) {
        eq = name.substr(loc + 1);
        name = name.substr(0, loc);
      }
      bool found = false;
      template for (constexpr auto member : reflection::nsdms(^^T)) {
        if (identifier_of(member) == name) {
          parser<typename[:decay(type_of(member)):]> p;
          if (!p.parse(state.[:member:], eq, rest)) {
            std::println("during parsing option: {:?}", curr);
            return false;
          }
          found = true;
          break;
        }
      }
      if (!found) {
        std::println("unrecognized option: {:?}", curr);
        return false;
      }
    }
    return true;
  }
};
} // namespace ivl::cmdline_parsing
