#pragma once

#include <ivl/exception>
#include <ivl/utility>
#include "variable_definition"
#include <set>
#include <string>
#include <string_view>
#include <optional>

namespace ivl::parsing::make {
struct state {
  std::set<variable_definition, variable_name_compare> variables;

  std::string dot_variable() const {
    std::string ret = ".VARIABLES";
    for (auto&& var : variables) ret += " " + var.name;
    return ret;
  }

  std::string expand_impl(std::string_view& text) const {
    contract_assert(text.starts_with("$("));
    text.remove_prefix(2);
    std::string ret;
    while (true) {
      text.empty() && panic("unterminated \"$(\"");
      if (text.starts_with(')')) {
        text.remove_prefix(1);
        break;
      }
      // TODO
      // if (text.starts_with(
    }
  }

  std::string expand(std::string_view text) const {
    auto text_copy = text;
    EXCEPTION_CONTEXT("text: {}", text_copy);
    EXCEPTION_CONTEXT("at: {}", text.data() - text_copy.data());
    std::string ret;
    auto take = [&](size_t n) {
      auto r = text.substr(0, n);
      text.remove_prefix(n);
      return r;
    };
    while (!text.empty()) {
      if (text.starts_with("$$")) {
        ret += "$";
        text.remove_prefix(2);
        continue;
      }
      if (text.starts_with("$(")) {
        text.remove_prefix(2);
        auto loc = text.find(')');
        loc == std::string_view::npos&& panic("missing closing paren");
        auto name = text.substr(0, loc);
        name.find('\n') == std::string_view::npos || panic("newline before closing paren: {:?}", text_copy);
        for (auto c : name) {
          c == ':' && panic("variables cannot have ':' in name");
          c == '#' && panic("variables cannot have '#' in name");
          c == '=' && panic("variables cannot have '=' in name");
          ::isspace(c) && panic("variables cannot have whitespace in name");
        }
        ret += get_variable(name);
        text.remove_prefix(loc + 1);
        continue;
      }
      text.starts_with("$") && todo();
      ret += take(1);
    }
    return ret;
  }

  std::string get_variable(std::string_view name) const {
    if (name == ".VARIABLES") return dot_variable();
    auto it = variables.find(name);
    if (it == variables.end()) return "";
    return it->contents;
  }

  std::optional<const variable_definition&> get_variable_2(std::string_view name) const {
    auto it = variables.find(name);
    if (it == variables.end()) return std::nullopt;
    return std::optional<const variable_definition&>{*it};
  }

  void set_variable_recursively_expanded(std::string_view name, std::string_view text) {
    name.empty() && panic("empty variable name");
    name.starts_with(".") && todo("special variable name: {:?}", name);
    auto it = variables.find(name);
    if (it != variables.end()) variables.erase(it);
    variables.insert(
      variable_definition{
        .name{name},
        .contents{text},
        .recursively_expanded = true,
        .overriden = false,
      }
    );
  }

  // void set_variable(const variable_definition& v) {
  //   v.name.starts_with(".") && todo("special variable name: {:?}", name);
  //   auto it = variables.find(v);
  //   if (it == variables.end()) {
  //     variables.insert(v);
  //     return;
  //   }
  //   if (it->overriden && !v.overriden) return;
  //   // TODO: maybe just mutate existing one
  //   variables.erase(it);
  //   variables.insert(v);
  // }
};
} // namespace ivl::parsing::make
