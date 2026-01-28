#pragma once

#include <ivl/cpp-parser/pp_tokens>

// https://marc.info/?l=boost&m=118835769257658&w=2

namespace ivl {

struct object_macro {
  std::string name;
  std::vector<pp_token> replacement_list;
};

struct function_macro {
  std::string_name;
  std::vector<std::string> arguments;
  bool variadic;
  // TODO: make it more structured
  std::vector<pp_token> replacement_list;

  // struct arg_token {
  //   size_t index;
  // };
  // struct vararg_token {};
  // struct varopt_token {
  //   std::vector<std::variant<pp_token, arg_token, vararg_token>> tokens;
  // };
  //
  // std::vector<std::variant<pp_token, arg_token, vararg_token, varopt_token>> replacement_list;
};

struct preprocessing_state {
  std::map<std::string, object_macro> object_macros;
  std::map<std::string, function_macro> function_macros;
  std::vector<pp_token> preprocessed;
  size_t counter = 0;
  size_t line = 1;

  bool is_whitespace(const pp_token& token) const {
    return std::holds_alternative<whitespace>(token.payload) ||
           std::holds_alternative<single_line_comment>(token.payload) ||
           std::holds_alternative<multi_line_comment>(token.payload);
  }

  bool is_pp_token(const pp_token& token) const {
    return !is_whitespace(token) && !std::holds_alternative<newline>(token.payload);
  }

  void skip_whitespace(std::span<const pp_token>& line) const {
    while (!line.empty() && is_whitespace(line.front())) line = line.subspan(1);
  }

  // bool try_consume(std::span<const pp_token>& line, const pp_token& token) {
  //   skip_whitespace(line);
  //   if (line.empty()) return false;
  //   if (line.front() != token) return false;
  //   line = line.subspan(1);
  //   return true;
  // }

  // bool is_endif(std::span<const pp_token> line) const {
  //   return try_consume(preprocessing_op_or_punc{"#"}) && try_consume(identifier{"endif"}) && try_consume(newline{});
  // }

  size_t find_newline(std::span<const pp_token> tokens) const {
    assert(!tokens.empty());
    assert(tokens.back() == pp_token{newline{}});
    for (size_t idx = 0; idx < tokens.size(); ++idx)
      if (tokens[idx] == pp_token{newline{}}) return idx;
    assert(false);
  }

  void handle_define(std::span<const pp_token>& tokens) {
    assert(!tokens.empty() && std::holds_alternative<newtokens>(tokens.back().payload));
    assert(tokens.front() == pp_token{identifier{"define"}});
    tokens = tokens.subspan(1);
    skip_whitespace(tokens);
    assert(std::holds_alternative<identifier>(tokens.front().payload));
    std::string name = std::get<identifier>(tokens.front().payload).text;
    tokens = tokens.subspan(1);
    if (auto ptr = std::get_if<preprocessing_op_or_punc>(&tokens.front().payload); ptr && ptr->kind == "(") {
      assert(false);
      // TODO: function macro
    } else {
      // object macro

      // https://eel.is/c++draft/cpp.replace#general-4
      assert(is_whitespace(tokens.front()));
      skip_whitespace(tokens);

      auto end = find_newline(tokens);

      object_macros[name] = object_macro{name, std::vector<pp_token>(tokens.data(), tokens.data() + end)};

      ++line;
      tokens = tokens.subspan(end + 1);
    }
  }

  void preprocess_line(std::span<const pp_token> line);

  void handle_preprocessing_directive(std::span<const pp_token>& tokens) {
    assert(!tokens.empty());
    assert(tokens.front() == pp_token{preprocessing_op_or_punc{"#"}});
    tokens = tokens.subspan(1);
    skip_whitespace(tokens);
    assert(!tokens.empty());
    if (tokens.front() == pp_token{newline{}}) {
      ++line;
      tokens = tokens.subspan(1);
      return;
    }
    if (tokens.front() == pp_token{identifier{"define"}}) {
      handle_define(tokens);
      return;
    }
    if (tokens.front() == pp_token{identifier{"include"}}) {
      handle_include(tokens);
      return;
    }
    if (tokens.front() == pp_token{identifier{"embed"}}) {
      assert(false);
    }
    if (tokens.front() == pp_token{identifier{"undef"}}) {
      handle_undef(tokens);
      return;
    }
    assert(false);
  }

  void preprocess(std::span<const pp_token> tokens) {
    if (tokens.empty()) return;
    assert(std::holds_alternative<newline>(tokens.back().payload));

    while (!tokens.empty()) {
      skip_whitespace(tokens);
      if (tokens.front() == pp_token{newline{}}) {
        ++line;
        tokens = tokens.subspan(1);
        continue;
      }

      if (tokens.front() == pp_token{preprocessing_op_or_punc{"#"}}) {
        handle_preprocessing_directive(tokens);
        continue;
      }

      assert(false && "TODO");
    }
  }
};

} // namespace ivl
