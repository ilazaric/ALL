#pragma once

#include <ivl/exception>
#include <ivl/linux/terminate_syscalls>
#include <ivl/linux/utility>
#include <ivl/parsing/basic_parser>
#include <ivl/reflection/test_attribute>
#include <ivl/stl/string>
#include <ivl/testing>
#include <ivl/utility>
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
  struct text {
    std::string value;
  };
  struct identifier {
    std::string value;
  };

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
  // TODO: validations
  std::vector<std::string> outputs;
  std::vector<std::string> implicit_outputs;
  std::vector<std::string> inputs;
  std::vector<std::string> implicit_inputs;
  std::vector<std::string> order_only_inputs;
  std::map<std::string, std::string> rule_vars; // including command
};

struct state {
  std::map<std::string, variable, std::less<>> variables;
  std::map<std::string, rule, std::less<>> rules;
  std::map<std::string, pool, std::less<>> pools;
  std::vector<build> builds;
  // TODO
};

struct include {
  std::string file;
};

inline void parse_file_into(const std::filesystem::path& file, state& global_state);

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

  std::string parse_expanded_text(std::string end_chars, const state& global_state) {
    std::string ret;
    consume_spaces();
    auto fin = [&] {
      for (auto c : end_chars)
        if (current_c() == c) return true;
      return false;
    };
    while (!fin()) {
      if (!consume_c_if('$')) {
        ret += current_c();
        consume_c_nocheck();
        continue;
      }
      if (consume_c_if('\n')) continue;
      if (consume_c_if(' ')) {
        ret += ' ';
        continue;
      }
      if (consume_c_if(':')) {
        ret += ':';
        continue;
      }
      if (consume_c_if('$')) {
        ret += '$';
        continue;
      }
      std::string_view id;
      if (consume_c_if('{')) {
        id = parse_identifier();
        consume_c('}');
      } else id = parse_identifier();
      ret += variable_value(id, global_state);
    }
    return ret;
  }

  variable parse_variable(const state& global_state) {
    variable ret;
    ret.name = std::string(parse_identifier());
    consume_spaces();
    consume_c('=');
    consume_spaces();
    ret.value = parse_expanded_text("\n", global_state);
    consume_c('\n');
    return ret;
  }

  rule_variable parse_rule_variable() {
    rule_variable ret;
    ret.name = std::string(parse_identifier());
    settable_rule_variables.contains(ret.name) || panic("illegal variable name");
    consume_spaces();
    consume_c('=');
    consume_spaces();

    // ret.value.emplace_back(rule_variable::text{});
    // auto last_text = &std::get<rule_variable::text>(ret.value.back());

    std::string last_text;

    // cant go through parse_expanded_text bc variables are not yet expanded
    while (!consume_c_if('\n')) {
      if (!consume_c_if('$')) {
        last_text += current_c();
        consume_c_nocheck();
        continue;
      }

      if (consume_c_if('\n')) continue;
      if (consume_c_if(' ')) {
        last_text += ' ';
        continue;
      }
      if (consume_c_if(':')) {
        last_text += ':';
        continue;
      }
      if (consume_c_if('$')) {
        last_text += '$';
        continue;
      }

      if (!last_text.empty()) {
        ret.value.emplace_back(rule_variable::text(last_text));
        last_text.clear();
      }

      std::string_view id;
      if (consume_c_if('{')) {
        id = parse_identifier();
        consume_c('}');
      } else id = parse_identifier();

      ret.value.emplace_back(rule_variable::identifier(std::string(id)));
    }

    if (!last_text.empty()) ret.value.emplace_back(rule_variable::text(last_text));

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

  build parse_build(state& global_state) {
    build ret;
    consume("build");
    consume_if(" ") || consume_if("$\n") || panic("malformed build declaration");
    consume_spaces();

    while (!consume_c_if(':')) {
      std::string output = parse_expanded_text(" :|\n", global_state);
      consume_spaces();
      current_c() == '\n' && panic("unexpected newline, expected ':'");
      ret.outputs.push_back(output);
      if (consume_c_if('|')) goto parse_implicit_outputs;
    }

    if (0) {
    parse_implicit_outputs:
      consume_spaces();
      while (!consume_c_if(':')) {
        std::string output = parse_expanded_text(" :|\n", global_state);
        consume_spaces();
        current_c() == '\n' && panic("unexpected newline, expected ':'");
        current_c() == '|' && panic("unexpected '|', expected ':'");
        ret.implicit_outputs.push_back(output);
      }
    }

    consume_spaces();
    std::string rulename(parse_identifier());
    rulename == "phony" || global_state.rules.contains(rulename) || panic("rule not found: {:?}", rulename);
    consume_spaces();
    while (!consume_c_if('\n')) {
      std::string input = parse_expanded_text(" |\n", global_state);
      consume_spaces();
      ret.inputs.push_back(input);
      if (consume_c_if('|')) {
        if (consume_c_if('|')) goto parse_order_only_inputs;
        else goto parse_implicit_inputs;
      }
    }

    if (0) {
    parse_implicit_inputs:
      consume_spaces();
      while (!consume_c_if('\n')) {
        std::string input = parse_expanded_text(" |\n", global_state);
        consume_spaces();
        ret.implicit_inputs.push_back(input);
        if (consume_c_if('|')) {
          consume_c('|');
          goto parse_order_only_inputs;
        }
      }
    }

    if (0) {
    parse_order_only_inputs:
      consume_spaces();
      while (!consume_c_if('\n')) {
        std::string input = parse_expanded_text(" |\n", global_state);
        current_c() == '|' && panic("unexpected '|', expected newline");
        consume_spaces();
        ret.order_only_inputs.push_back(input);
      }
    }

    std::map<std::string, std::string> local_vars;
    while (!finished()) {
      if (current_c() == '#') {
        while (!consume_c_if('\n')) consume_c_nocheck();
        continue;
      }
      if (current_c() != ' ') break;
      consume_spaces();
      std::string var(parse_identifier());
      consume_spaces();
      consume_c('=');
      consume_spaces();
      std::string text = parse_expanded_text("\n", global_state);
      consume_c('\n');
      local_vars[var] = text;
    }

    auto join = [](const std::vector<std::string>& vec) -> std::string {
      if (vec.empty()) return "";
      auto ret = vec[0];
      for (std::size_t i = 1; i < vec.size(); ++i) ret += " " + vec[i];
      return ret;
    };
    local_vars["out"] = join(ret.outputs);
    local_vars["in"] = join(ret.inputs);

    if (rulename == "phony") return ret;

    for (auto&& [outer_id, pieced_text] : global_state.rules.at(rulename).variables) {
      std::string text;
      for (auto&& el : pieced_text.value) {
        if (auto ptr = std::get_if<rule_variable::text>(&el)) {
          text += ptr->value;
          continue;
        }
        if (auto ptr = std::get_if<rule_variable::identifier>(&el)) {
          auto&& inner_id = ptr->value;
          if (local_vars.contains(inner_id)) text += local_vars.at(inner_id);
          else if (global_state.variables.contains(inner_id)) text += global_state.variables.at(inner_id).value;
          // unset is not an error, expands to empty string
          continue;
        }
        panic("shouln't be reachable");
      }
      ret.rule_vars[outer_id] = text;
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

  include parse_include(state& global_state) {
    include ret;
    consume("include");
    consume_if(" ") || consume_if("$\n") || panic("malformed include declaration");
    consume_spaces();
    while (true) {
      if (current_c() == ' ') break;
      if (current_c() == '\n') break;
      if (!consume_c_if('$')) {
        if (current_c() != '\n') ret.file += current_c();
        consume_c_nocheck();
        continue;
      }
      if (consume_c_if(' ')) {
        ret.file += ' ';
        continue;
      }
      if (consume_c_if('$')) {
        ret.file += '$';
        continue;
      }
      if (consume_c_if('$')) {
        ret.file += '$';
        continue;
      }
      std::string_view id;
      if (consume_c_if('{')) {
        id = parse_identifier();
        consume_c('}');
      } else id = parse_identifier();
      ret.file += variable_value(id, global_state);
    }
    consume_spaces();
    if (consume_c_if('#'))
      while (current_c() != '\n') consume_c_nocheck();
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
      r.name == "phony" && panic("rule can't be called \"phony\"");
      global_state.rules.contains(r.name) && panic("duplicate rule {:?}", r.name);
      global_state.rules[r.name] = std::move(r);
      return;
    }

    // TODO: console??
    if (starts_with_and_space("pool")) {
      auto p = parse_pool();
      global_state.pools.contains(p.name) && panic("duplicate pool {:?}", p.name);
      global_state.pools[p.name] = p;
      return;
    }

    if (starts_with_and_space("build")) {
      global_state.builds.push_back(parse_build(global_state));
      return;
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
      auto i = parse_include(global_state);
      parse_file_into(i.file, global_state);
      return;
    }

    // should be variable declaration
    auto v = parse_variable(global_state);
    global_state.variables[v.name] = v;
  }
};

inline void parse_text_into(std::string_view contents, state& global_state) {
  if (contents.empty()) return;
  contents.ends_with("\n") || panic("file does not terminate with newline");
  parser parser(contents);
  EXCEPTION_CONTEXT("parser state -- {}", parser.debug_context());
  while (!parser.finished()) parser.parse_declaration(global_state);
}

inline void parse_file_into(const std::filesystem::path& file, state& global_state) {
  auto contents = linux::read_file(file);
  parse_text_into(contents, global_state);
}

// default: build.ninja in current working directory
inline state parse(const std::filesystem::path& file) {
  state global_state;
  parse_file_into(file, global_state);
  return global_state;
}

// IVL has_test_variant()

[[= ivl::test]] inline void test_variables() {
  std::string_view text = R"ninja(
a=A
b=$a$a$a
a=B
c=$b${a}C
)ninja";
  state global_state;
  parse_text_into(text, global_state);
  contract_assert(global_state.variables.size() == 3);
  contract_assert(global_state.variables.at("a").value == "B");
  contract_assert(global_state.variables.at("b").value == "AAA");
  contract_assert(global_state.variables.at("c").value == "AAABC");
}

[[= ivl::test]] inline void test_rules() {
  std::string_view text = R"ninja(
rule touch
  command = touch $out
)ninja";
  state global_state;
  parse_text_into(text, global_state);
  testing::contract_assert_json(global_state, R"json(
    {
      "builds": [],
      "pools": {},
      "rules": {
        "touch": {
          "name": "touch",
          "variables": {
            "command": {
              "name": "command",
              "value": [
                {
                  "type": "ivl::parsing::ninja::rule_variable::text",
                  "value": {
                    "value": "touch "
                  }
                },
                {
                  "type": "ivl::parsing::ninja::rule_variable::identifier",
                  "value": {
                    "value": "out"
                  }
                }
              ]
            }
          }
        }
      },
      "variables": {}
    }
  )json");
}

[[= ivl::test]] inline void test_pool() {
  std::string_view text = R"ninja(
pool gpu
  depth = 67
)ninja";
  state global_state;
  parse_text_into(text, global_state);
  contract_assert(global_state.pools.size() == 1);
  auto& gpu = global_state.pools.at("gpu");
  contract_assert(gpu.name == "gpu");
  contract_assert(gpu.depth == 67);
}

[[= ivl::test]] inline void test_include() {
  namespace sys = ::ivl::linux::terminate_syscalls;
  linux::owned_file_descriptor root(sys::memfd_create("root", 0));
  linux::owned_file_descriptor mid(sys::memfd_create("mid", 0));
  linux::owned_file_descriptor leaf(sys::memfd_create("leaf", 0));
  linux::write_file_slow(root, std::format("include /proc/self/fd/{}\n", mid.get()));
  linux::write_file_slow(mid, std::format("include /proc/self/fd/{}\n", leaf.get()));
  linux::write_file_slow(leaf, "abc=def\n");
  state global_state;
  parse_file_into(std::format("/proc/self/fd/{}", root.get()), global_state);
  contract_assert(global_state.variables.at("abc").value == "def");
}

[[= ivl::test]] inline void test_build() {
  std::string_view text = R"ninja(
rule touch
  command = touch $out

build foo: touch
)ninja";
  state global_state;
  parse_text_into(text, global_state);
  testing::contract_assert_json(global_state.builds, R"json(
    [{
      "inputs": [],
      "outputs": [
        "foo"
      ],
      "implicit_outputs": [],
      "implicit_inputs": [],
      "order_only_inputs": [],
      "rule_vars": {
        "command": "touch foo"
      }
    }]
  )json");
}

[[= ivl::test]] inline void test_phony() {
  std::string_view text = R"ninja(
build bar: phony foo
)ninja";
  state global_state;
  parse_text_into(text, global_state);
  testing::contract_assert_json(global_state.builds, R"json(
    [{
      "inputs": [
        "foo"
      ],
      "outputs": [
        "bar"
      ],
      "implicit_outputs": [],
      "implicit_inputs": [],
      "order_only_inputs": [],
      "rule_vars": {}
    }]
  )json");
}

[[= ivl::test]] inline void test_build_io() {
  std::string_view text = R"ninja(
build a | b: phony c | d || e
)ninja";
  state global_state;
  parse_text_into(text, global_state);
  testing::contract_assert_json(global_state.builds, R"json(
    [{
      "inputs": [
        "c"
      ],
      "outputs": [
        "a"
      ],
      "implicit_outputs": [
        "b"
      ],
      "implicit_inputs": [
        "d"
      ],
      "order_only_inputs": [
        "e"
      ],
      "rule_vars": {}
    }]
  )json");
}
} // namespace ivl::parsing::ninja
