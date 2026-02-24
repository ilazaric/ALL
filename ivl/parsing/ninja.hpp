#pragma once

#include <ivl/exception>
#include <ivl/linux/utility>
#include <ivl/parsing/basic_parser>
#include <ivl/stl/string>
#include <ivl/util>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// https://ninja-build.org/manual.html#ref_ninja_file
namespace ivl::parsing::ninja {
struct variable {
  std::string name;
  std::string value;
};

struct rule_variable {
  struct text : std::string {};
  struct identifier : std::string {};

  std::string name;
  std::vector<std::variant<text, identifier>> value;
};

// depfile, deps, command, pool
struct rule {
  std::string name;
  std::map<std::string, rule_variable> variables;
};

struct pool {
  std::string name;
  size_t depth;
};

struct build {
  std::vector<std::string> outputs;
  std::string rulename;
  std::vector<std::string> inputs;
  std::vector<variable> variables;
};

struct state {
  std::map<std::string, variable, std::less<>> variables;
  std::map<std::string, rule, std::less<>> rules;
  std::map<std::string, pool, std::less<>> pools;
  // TODO
};

struct parser : basic_parser {
  using basic_parser::basic_parser;

  std::set<std::string> settable_rule_variables{
    "command",   "depfile", "deps",    "msvc_deps_prefix", "description", "dyndep",
    "generator", "restat",  "rspfile", "rspfile_content",  "pool",
  };

  void consume_spaces() { while (consume_c_if(' ') || consume_if("$\n")); }

  bool is_identifier_c(char c) {
    if (c >= 'a' && c <= 'z') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == '-' || c == '_') return true;
    return false;
  }

  std::string_view parse_identifier() {
    auto start = &current_c();
    while (is_identifier_c(current_c())) consume_c_nocheck();
    auto end = &current_c();
    start == end&& panic("malformed identifier");
    return std::string_view(start, end);
  }

  // TODO: need to understand subninja and include
  // TODO: maybe could return string_view
  std::string variable_value(std::string_view id, const state& global_state) {
    return global_state.variables.contains(id) ? global_state.variables.find(id)->second.value : "";
  }

  variable parse_variable(const state& global_state) {
    variable ret;
    ret.name = std::string(parse_identifier());
    consume_spaces();
    consume_c('=');
    consume_spaces();

    while (!consume_c_if('\n')) {
      if (!consume_c_if('$')) {
        ret.value += current_c();
        consume_c_nocheck();
        continue;
      }

      if (consume_c_if('\n')) continue;
      if (consume_c_if(' ')) {
        ret.value += ' ';
        continue;
      }
      if (consume_c_if(':')) {
        ret.value += ':';
        continue;
      }
      if (consume_c_if('$')) {
        ret.value += '$';
        continue;
      }

      std::string_view id;
      if (consume_c_if('{')) {
        id = parse_identifier();
        consume_c('}');
      } else id = parse_identifier();

      ret.value += variable_value(id, global_state);
    }

    return ret;
  }

  rule_variable parse_rule_variable() {
    rule_variable ret;
    ret.name = std::string(parse_identifier());
    settable_rule_variables.contains(ret.name) || panic("illegal variable name");
    consume_spaces();
    consume_c('=');
    consume_spaces();

    ret.value.emplace_back(rule_variable::text{});
    auto last_text = &std::get<rule_variable::text>(ret.value.back());

    while (!consume_c_if('\n')) {
      if (!consume_c_if('$')) {
        *last_text += current_c();
        consume_c_nocheck();
        continue;
      }

      if (consume_c_if('\n')) continue;
      if (consume_c_if(' ')) {
        *last_text += ' ';
        continue;
      }
      if (consume_c_if(':')) {
        *last_text += ':';
        continue;
      }
      if (consume_c_if('$')) {
        *last_text += '$';
        continue;
      }

      std::string_view id;
      if (consume_c_if('{')) {
        id = parse_identifier();
        consume_c('}');
      } else id = parse_identifier();

      ret.value.emplace_back(rule_variable::identifier(std::string(id)));
      ret.value.emplace_back(rule_variable::text{});
      last_text = &std::get<rule_variable::text>(ret.value.back());
    }

    return ret;
  }

  rule parse_rule() {
    rule ret;
    consume("rule");
    consume_if(" ") || consume_if("$\n") || panic("malformed rule declaration");
    consume_spaces();
    ret.name = std::string(parse_identifier());
    consume_spaces();
    consume_c('\n');

    while (!finished()) {
      if (current_c() == '#') {
      comment:
        while (current_c() != '\n') consume_c_nocheck();
        consume_c('\n');
        continue;
      }
      if (current_c() != ' ') break;
      while (consume_c_if(' '));
      if (current_c() == '#') goto comment;
      if (consume_c_if('\n')) break;
      auto var = parse_rule_variable();
      ret.variables[var.name] = var;
    }

    return ret;
  }

  pool parse_pool() {
    pool ret;
    consume("pool");
    consume_if(" ") || consume_if("$\n") || panic("malformed pool declaration");
    consume_spaces();
    ret.name = std::string(parse_identifier());
    consume_spaces();
    consume_c('\n');

    consume_c(' ');
    consume_spaces();
    consume("depth");
    consume_spaces();
    consume_c('=');
    consume_spaces();
    current_c() >= '1' && current_c() <= '9' || panic("malformed integer");
    ret.depth = current_c() - '0';
    consume_c_nocheck();
    while (current_c() >= '0' && current_c() <= '9') {
      __builtin_mul_overflow(ret.depth, 10, &ret.depth) && panic("integer too big");
      __builtin_add_overflow(ret.depth, current_c() - '0', &ret.depth) && panic("integer too big");
      consume_c_nocheck();
    }
    consume_spaces();
    consume_c('\n');

    return ret;
  }

  void parse_declaration(state& global_state) {
    if (consume_c_if('\n')) return;
    current_sv().starts_with("$\n") && panic("lexing error");

    if (consume_c_if('#')) {
    comment:
      while (current_c() != '\n') consume_c_nocheck();
      consume_c('\n');
      return;
    }

    if (consume_c_if(' ')) {
      consume_spaces();
      if (consume_c_if('#')) goto comment;
      consume_c('\n');
      return;
    }

    auto starts_with_and_space = [&](std::string_view sv) {
      auto curr = current_sv();
      if (!curr.starts_with(sv)) return false;
      curr.remove_prefix(sv.size());
      return curr.starts_with(' ') || curr.starts_with('$');
    };

    if (starts_with_and_space("rule")) {
      auto r = parse_rule();
      global_state.rules.contains(r.name) && panic("duplicate rule {:?}", r.name);
      global_state.rules[r.name] = std::move(r);
      return;
    }

    // console??
    if (starts_with_and_space("pool")) {
      auto p = parse_pool();
      global_state.pools.contains(p.name) && panic("duplicate pool {:?}", p.name);
      global_state.pools[p.name] = p;
      return;
    }

    if (starts_with_and_space("build")) {
      todo();
    }

    if (starts_with_and_space("default")) {
      todo();
    }

    // variable
    // if (line.starts_with("ninja_required_version ")) {
    //   todo();
    // }

    // variable
    // if (line.starts_with("msvc_deps_prefix ")) {
    //   todo();
    // }

    if (starts_with_and_space("subninja")) {
      todo();
    }

    if (starts_with_and_space("include")) {
      todo();
    }

    // should be variable declaration
    auto v = parse_variable(global_state);
    global_state.variables[v.name] = v;
  }
};

// default: build.ninja in current working directory
state parse(const std::filesystem::path& file) {
  auto contents = linux::read_file(file);
  if (contents.empty()) return state{};
  contents.ends_with("\n") || panic("file does not terminate with newline");

  parser parser(contents);
  EXCEPTION_CONTEXT("parser state -- {}", parser.debug_context());

  state global_state;

  // auto parse_build = [&] {
  //   build ret;
  //   consume("build");
  //   consume_if(" ") || consume_if("$\n") || panic("malformed build declaration");
  //   consume_spaces();

  //   while (!consume_if_c(':')) {
  //     ret.outputs.emplace_back(parse_identifier());
  //     consume_spaces();
  //   }

  //   consume_spaces();
  //   ret.rulename = std::string(parse_identifier());
  //   consume_spaces();

  //   while (current_c() != '\n' && current_c() != '|') {
  //   }
  // };

  while (!parser.finished()) parser.parse_declaration(global_state);

  return global_state;
}
} // namespace ivl::parsing::ninja
