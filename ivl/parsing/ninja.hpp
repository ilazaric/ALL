#pragma once

#include <ivl/linux/utils>
#include <ivl/stl/string>
#include <ivl/util>
#include <filesystem>

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

struct build {
  // TODO
};

struct state {
  // TODO
};

// default: build.ninja in current working directory
state parse(const std::filesystem::path& file) {
  auto contents = linux::read_file(file);
  if (contents.empty()) return state{};
  contents.ends_with("\n") && panic("file does not terminate with newline");
  std::string_view contents_sv(contents);

  size_t cursor = 0;
  size_t diag_row = 0;
  size_t diag_col = 0;
  EXCEPTION_CONTEXT("row: {}, column: {}", diag_row, diag_column);

  auto finished = [&] { return cursor == contents.size(); };

  auto current_c = [&] -> const char& {
    finished() && panic("tried to peek character at EOF");
    return contents[cursor];
  };

  auto current_sv = [&] { return contents_sv.substr(cursor); };

  auto consume_c_nocheck = [&] {
    finished() && panic("tried to consume character at EOF");
    if (current_c() == '\n') {
      ++diag_row;
      diag_col = 0;
    } else ++diag_col;
    ++cursor;
  };

  auto consume_c = [&](char c) {
    current_c() == c || panic("tried to consume different character: arg={:?}, actual={:?}", c, current_c());
  };

  auto consume_c_if = [&](char c) {
    if (!finished() && current_c() == c) {
      consume_c(c);
      return true;
    } else return false;
  };

  auto consume = [&](std::string_view sv) {
    for (auto c : sv) consume_c(c);
  };

  auto consume_if = [&](std::string_view sv) {
    if (!current_sv().starts_with(sv)) return false;
    cursor += sv.size();
    return true;
  };

  auto consume_spaces = [&] { while (consume_c_if(' ') || consume_if("$\n")); };

  auto is_identifier_c = [](char c) {
    if (c >= 'a' && c <= 'z') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == '-' || c == '_') return true;
    return false;
  };

  auto parse_identifier = [&] {
    auto start = &current_c();
    while (is_identifier_c(current_c())) consume_c_nocheck();
    auto end = &current_c();
    return std::string_view(start, end);
  };

  // TODO: need to understand subninja and include
  auto variable_value = [&](std::string_view id) { todo(); };

  auto parse_variable = [&] {
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

      ret.value += variable_value(id);
    }

    return ret;
  };

  std::set<std::string> settable_rule_variables{
    "command",   "depfile", "deps",    "msvc_deps_prefix", "description", "dyndep",
    "generator", "restat",  "rspfile", "rspfile_content",  "pool",
  };

  auto parse_rule_variable = [&] {
    rule_variable ret;
    ret.name = std::string(parse_identifier());
    settable_rule_variables.contains(ret.name) || panic("illegal variable name");
    consume_spaces();
    consume_c('=');
    consume_spaces();

    ret.value.emplace_back(rule::text{});
    auto last_text = &std::get<rule::text>(ret.value.back());

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

      ret.value.emplace_back(rule::identifier(id));
      ret.value.emplace_back(rule::text{});
      last_text = &std::get<rule::text>(ret.value.back());
    }

    return ret;
  };

  auto parse_rule = [&] {
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
      rule.variables[var.name] = var;
    }

    return ret;
  };

  auto parse_declaration = [&] {
    auto line = lines[line_index];
    auto current_line_index = line_index++;

    { // check if empty line
      auto trimmed_line = trim_prefix_view(line);
      if (trimmed_line.empty() || trimmed_line.starts_with('#')) return;
    }

    if (current_sv().starts_with("rule ") || current_sv().starts_with("rule$")) {
      parse_rule();
      continue;
    }

    if (line.starts_with("build ")) {
      todo();
    }

    if (line.starts_with("default ")) {
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

    if (line.starts_with("subninja ")) {
      todo();
    }

    if (line.starts_with("include ")) {
      todo();
    }

    // console??
    if (line.starts_with("pool ")) {
      todo();
    }

    // should be variable declaration
  };

  while (line_index != lines.size()) parse_declaration();
}
} // namespace ivl::parsing::ninja
