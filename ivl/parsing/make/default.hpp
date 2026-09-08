#pragma once

#include <ivl/linux/utility>
#include <ivl/utility>
#include "logical_parser"
#include "state"
#include <map>
#include <set>
#include <string>
#include <string_view>

// https://www.gnu.org/software/make/manual/html_node/Makefile-Contents.html

namespace ivl::parsing::make {
struct explicit_rule;
struct implicit_rule;
struct variable_definition;
struct directive;
struct comment;

// struct explicit_rule {
//   std::string target;
//   std::vector<std::string> prerequisites;
//   std::string recipe;
// };

struct parser : logical_parser {
  using logical_parser::logical_parser;

  bool consume_if_ws() {
    if (finished()) return false;
    if (current_c() == '\n') return false;
    if (!::isspace(current_c())) return false;
    consume_c_nocheck();
    return true;
  }

  void consume_ws() { consume_if_ws() || panic("expected non-newline whitespace, got {:?}", current_c()); }

  void consume_while_ws() { while (consume_if_ws()); }

  std::string_view consume_logical_line() {
    auto start = get_cursor();
    while (!finished()) {
      // if (consume_if("\\\n")) continue;
      // if (consume_if("\\\\")) continue;
      if (consume_c_if('\n')) break;
      consume_c_nocheck();
    }
    return slice(start, get_cursor());
  }

  void parse_comment() {
    consume_c('#');
    consume_logical_line();
  }

  bool try_parse_comment() {
    if (finished()) return false;
    if (current_c() != '#') return false;
    parse_comment();
    return true;
  }

  void stupid_consume_dollar_paren() {
    consume("$(");
    size_t depth = 1;
    while (depth) {
      if (consume_if("$(")) {
        ++depth;
        continue;
      }
      if (consume_if(")")) {
        --depth;
        continue;
      }
      current_c() == '\n' && panic("unexpected newline");
      consume_c_nocheck();
    }
  }

  std::string stupid_expand(std::string_view text, state& global_state) {
    std::string ret;
    while (!text.empty()) {
      if (!text.starts_with("$(")) {
        ret += text.front();
        text.remove_prefix(1);
        continue;
      }
      text.remove_prefix(2);
      auto loc = text.find(')');
      loc == std::string_view::npos&& panic("missing closing paren: {:?}", text);
      auto name = text.substr(0, loc);
      name.find('\n') == std::string_view::npos || panic("newline before closing paren: {:?}", text);
      for (auto c : name) {
        c == ':' && panic("variables cannot have ':' in name");
        c == '#' && panic("variables cannot have '#' in name");
        c == '=' && panic("variables cannot have '=' in name");
        ::isspace(c) && panic("variables cannot have whitespace in name");
      }
      ret += global_state.get_variable(name);
      text.remove_prefix(loc + 1);
    }
    return ret;
  }

  bool try_parse_ifeq(state& global_state) {
    bool neg;
    {
      auto cp = get_checkpoint();
      neg = consume_if("ifneq");
      if (!neg && !consume_if("ifeq")) return false;
      if (!consume_if_ws()) {
        restore_from_checkpoint(cp);
        return false;
      }
    }
    consume_while_ws();
    current_c() == '\'' && todo();
    current_c() == '"' && todo();
    // ifeq ( arg1 , arg2 )
    consume_c('(');
    std::string_view arg1, arg2;
    {
      auto arg1_begin = get_cursor();
      while (!consume_c_if(',')) {
        current_c() == '\n' && panic("malformed ifeq");
        if (current_sv().starts_with("$(")) stupid_consume_dollar_paren();
        else consume_c_nocheck();
      }
      auto arg1_end = get_cursor() - 1;
      auto arg2_begin = get_cursor();
      while (!consume_c_if(')')) {
        current_c() == '\n' && panic("malformed ifeq");
        if (current_sv().starts_with("$(")) stupid_consume_dollar_paren();
        else consume_c_nocheck();
      }
      auto arg2_end = get_cursor() - 1;
      arg1 = slice(arg1_begin, arg1_end);
      arg2 = slice(arg2_begin, arg2_end);
      while (!arg1.empty() && ::isspace(arg1.back())) arg1.remove_suffix(1);
      while (!arg2.empty() && ::isspace(arg2.front())) arg2.remove_prefix(1);
    }
    consume_while_ws();
    if (!try_parse_comment()) consume_c('\n');

    bool test = neg != (stupid_expand(arg1, global_state) == stupid_expand(arg2, global_state));
    test&& todo();

    while (true) {
      consume_if("else") && todo();
      if (consume_if("endif")) {
        if (try_parse_comment() || consume_c_if('\n')) break;
        if (!consume_if_ws()) {
          while (current_c() != '\n') consume_c_nocheck();
          consume_c('\n');
          continue;
        }
        consume_while_ws();
        if (!try_parse_comment()) consume_c('\n');
        break;
      }
      while (current_c() != '\n') consume_c_nocheck();
      consume_c('\n');
    }

    return true;
  }

  bool try_parse_ifdef(state& global_state) {
    if (!current_sv().starts_with("ifdef ") && !current_sv().starts_with("ifndef ")) return false;
    todo();
  }

  bool try_parse_conditional_directive(state& global_state) {
    return try_parse_ifeq(global_state) || try_parse_ifdef(global_state);
  }

  bool try_parse_variable_assignment(state& global_state) {
    auto cp = get_checkpoint();
    // bool revert = true;
    // util::scope_exit _{[&]{ if (revert) restore_from_checkpoint(cp); }};
    consume_while_ws();
    if (finished()) return (restore_from_checkpoint(cp), false);
    if (current_c() == '\n') return (restore_from_checkpoint(cp), false);
    if (current_c() == '#') return (restore_from_checkpoint(cp), false);
    if (current_c() == ':') return (restore_from_checkpoint(cp), false);
    auto name_begin = get_cursor();
    auto name_end = get_cursor();
    std::string_view op;
    auto try_parse_op = [&] {
      auto begin = get_cursor();
      if (
        consume_if("=") || consume_if("+=") || consume_if(":=") || consume_if("::=") || consume_if("?=") ||
        consume_if("!=")
      ) {
        op = slice(begin, get_cursor());
        return true;
      } else {
        return false;
      }
    };
    while (true) {
      if (finished()) return (restore_from_checkpoint(cp), false);
      if (::isspace(current_c())) break;
      if (auto e = get_cursor(); try_parse_op()) {
        name_end = e;
        break;
      }
      if (current_c() == '#') return (restore_from_checkpoint(cp), false);
      if (current_c() == '\n') return (restore_from_checkpoint(cp), false);
      if (current_c() == ':') return (restore_from_checkpoint(cp), false);
      consume_c_nocheck();
    }
    if (name_end == name_begin) name_end = get_cursor();
    auto name = slice(name_begin, name_end);
    name.empty() && panic("empty variable name");
    consume_while_ws();
    if (op.empty() && !try_parse_op()) return (restore_from_checkpoint(cp), false);
    consume_while_ws();
    std::string text;
    while (!finished()) {
      if (consume_c_if('\n')) break;
      if (consume_if("\\\\")) {
        text += "\\";
        continue;
      }
      if (consume_if("\\#")) {
        text += "#";
        continue;
      }
      if (current_sv() == "\\") {
        text += "\\";
        consume_c('\\');
        break;
      }
      current_c() == '\\' && todo();
      if (try_parse_comment()) break;
      text += current_c();
      consume_c_nocheck();
    }
    while (!text.empty() && ::isspace(text.back())) text.pop_back();
    if (op == "=") {
      global_state.set_variable_recursively_expanded(name, text);
    } else {
      todo();
    }
    return true;
  }

  bool try_parse_override_directive(state& global_state) {
    auto cp = get_checkpoint();
    consume_while_ws();
    if (!consume_if("override")) return (restore_from_checkpoint(cp), false);
    if (!consume_if_ws()) return (restore_from_checkpoint(cp), false);
    consume_while_ws();
    try_parse_variable_assignment(global_state) || panic("broken override directive");
    return true;
  }

  void parse_something(state& global_state) {
    if (consume_c_if('\n')) return;
    if (current_c() == '#') return parse_comment();
    if (try_parse_conditional_directive(global_state)) return;
    if (try_parse_override_directive(global_state)) return;
    if (try_parse_variable_assignment(global_state)) return;
    todo();
  }
};

inline state parse_text(std::string_view contents) {
  state global_state;
  parser parser(contents);
  // TODO: this should be in destructor of basic_parser probably
  EXCEPTION_CONTEXT("file snippet:\n{}", parser.debug_context_file(5));
  EXCEPTION_CONTEXT("parser state -- {}", parser.debug_context());
  while (!parser.finished()) parser.parse_something(global_state);
  return global_state;
}

inline state parse(const std::filesystem::path& file) {
  EXCEPTION_CONTEXT("file: {}", file);
  auto contents = linux::read_file(file);
  return parse_text(contents);
}
} // namespace ivl::parsing::make
