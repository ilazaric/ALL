#pragma once

#include <ivl/linux/utility>
#include "../basic_parser"
#include <map>
#include <string>
#include <string_view>

namespace ivl::parsing::make {
// https://www.gnu.org/software/make/manual/html_node/Makefile-Contents.html
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

struct variable_definition {
  std::string name;
  std::string contents;
  bool recursively_expanded;
  bool overriden; // either passed via cmdline (make A=1), or override A=1
};

struct variable_name_compare {
  using is_transparent = void;
  static std::string_view name(const variable_name_compare& v) { return a.name; }
  static std::string_view name(const std::string& s) { return s; }
  static std::string_view name(std::string_view sv) { return sv; }
  static bool operator()(const auto& a, const auto& b) { return name(a) < name(b); }
};

struct state {
  std::set<variable_definition, variable_name_compare> variables;
  // std::map<std::string, std::string, std::less<>> variables;

  std::string dot_variable() const {
    std::string ret = ".VARIABLES";
    for (auto&& var : variables) ret += " " + var.name;
    return ret;
  }

  std::string expand_impl(std::string_view& text) {
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

  std::string expand(std::string_view text) {
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
    auto& var = *it;
    return var.recursively_expanded ? expand(var.contents) : var.contents;
  }

  void set_variable_recursively_expanded(std::string_view name, std::string_view text) {
    name.empty() && panic("empty variable name");
    name.starts_with(".") && todo("special variable name: {:?}", name);
    variables[name] = text;
  }
};

struct logical_parser {
  std::string logical_contents;
  basic_parser logical_parser;
  basic_parser physical_parser;
  bool escaped = false;

  struct checkpoint {
    basic_parser::checkpoint logical;
    basic_parser::checkpoint physical;
  };

  checkpoint get_checkpoint() const { return {logical_parser.get_checkpoint(), physical_parser.get_checkpoint()}; }

  void restore_from_checkpoint(checkpoint c) {
    logical_parser.restore_from_checkpoint(c.logical);
    physical_parser.restore_from_checkpoint(c.physical);
  }

  logical_parser(std::string_view contents) : logical_parser(contents), physical_parser(contents) {
    while (!contents.empty()) {
      if (contents.starts_with("\\\n")) {
        contents.remove_prefix(2);
        continue;
      }
      if (contents.starts_with("\\\\")) {
        contents.remove_prefix(2);
        logical_contents += "\\\\";
        continue;
      }
      logical_contents += contents[0];
      contents.remove_prefix(1);
    }
    logical_parser = basic_parser(logical_contents);
  }

  bool finished() const { return logical_parser.finished(); }

  decltype(auto) current_sv() const { return logical_parser.current_sv(); }
  decltype(auto) current_c() const { return logical_parser.current_c(); }
  decltype(auto) slice(size_t lo, size_t hi) const { return logical_parser.slice(lo, hi); }

  decltype(auto) debug_context() const { return physical_parser.debug_context(); }
  decltype(auto) debug_context_file(size_t count) const { return physical_parser.debug_context_file(count); }

  void consume_c_nocheck() {
    if (escaped) {
      physical_parser.consume_c_nocheck();
      logical_parser.consume_c_nocheck();
      while (physical_parser.consume_if("\\\n"));
      escaped = false;
      return;
    }
    if (current_sv().starts_with("\\\\")) {
      physical_parser.consume_c_nocheck();
      logical_parser.consume_c_nocheck();
      escaped = true;
      return;
    }
    physical_parser.consume_c_nocheck();
    logical_parser.consume_c_nocheck();
    while (physical_parser.consume_if("\\\n"));
  }

  void consume_c(char c) {
    current_c() == c || panic("tried to consume different character: arg={:?}, actual={:?}", c, current_c());
    consume_c_nocheck();
  }

  bool consume_c_if(char c) {
    if (!finished() && current_c() == c) {
      consume_c(c);
      return true;
    } else return false;
  }

  void consume(std::string_view sv) {
    for (auto c : sv) consume_c(c);
  }

  bool consume_if(std::string_view sv) {
    if (!current_sv().starts_with(sv)) return false;
    consume(sv);
    return true;
  }

  size_t get_cursor() const { return logical_parser.cursor; }
};

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
      if (consume_c_if("\n")) break;
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
    std::string_view op;
    auto try_parse_op = [&] {
      auto begin = get_cursor();
      if (
        consume_if("=") || consume_if("+=") || consume_if(":=") || consume_if("::=") || consume_if("?=") ||
        consume_if("!=")
      )
        return false;
      op = slice(begin, get_cursor());
      return true;
    };
    while (true) {
      if (finished()) return (restore_from_checkpoint(cp), false);
      if (::isspace(current_c())) break;
      if (try_parse_op()) {
      }
      if (current_c() == '=') break;
      if (current_sv().starts_with("+=")) break;
      if (current_sv().starts_with(":=")) break;
      if (current_sv().starts_with("::=")) break;
      if (current_c() == '#') return (restore_from_checkpoint(cp), false);
      if (current_c() == '\n') return (restore_from_checkpoint(cp), false);
      if (current_c() == ':') return (restore_from_checkpoint(cp), false);
      consume_c_nocheck();
    }
    auto name_end = get_cursor();
    auto name = slice(name_begin, name_end);
    consume_while_ws();
    bool plus_eq = consume_if("+=");
    if (!plus_eq && !consume_c_if('=') && !consume_if(":=") && !consume_if("::="))
      return (restore_from_checkpoint(cp), false);
    name.empty() && panic("empty variable name");
    std::string text;
    while (!finished()) {
      if (consume_c_if('\n')) break;
      // if (consume_if("\\\n")) continue;
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
    if (plus_eq) {
      global_state.set_variable_recursively_expanded(name, global_state.get_variable(name) + text);
    } else {
      global_state.set_variable_recursively_expanded(name, text);
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

inline state parse(const std::filesystem::path& file) {
  state global_state;
  auto contents = linux::read_file(file);
  parser parser(contents);
  // TODO: this should be in destructor of basic_parser probably
  EXCEPTION_CONTEXT("file snippet:\n{}", parser.debug_context_file(5));
  EXCEPTION_CONTEXT("parser state -- {}", parser.debug_context());
  while (!parser.finished()) parser.parse_something(global_state);
  return global_state;
}
} // namespace ivl::parsing::make
