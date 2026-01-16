#pragma once

#include <ivl/cpp-parser/pp_tokens>

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

  bool is_whitespace(const pp_token& token) const {
    return std::holds_alternative<whitespace>(token.payload) ||
           std::holds_alternative<single_line_comment>(token.payload) ||
           std::holds_alternative<multi_line_comment>(token.payload);
  }

  void skip_whitespace(std::span<const pp_token>& line) const {
    while (!line.empty() && is_whitespace(line.front())) line = line.subspan(1);
  }

  bool try_consume(std::span<const pp_token>& line, const pp_token& token) {
    skip_whitespace(line);
    if (line.empty()) return false;
    if (line.front() != token) return false;
    line = line.subspan(1);
    return true;
  }

  bool is_endif(std::span<const pp_token> line) const {
    return try_consume(preprocessing_op_or_punc{"#"}) && try_consume(identifier{"endif"}) && try_consume(newline{});
  }

  void handle_define(std::span<const pp_token> line) {
    assert(!line.empty() && std::holds_alternative<newline>(line.back().payload));
    assert(try_consume(line, preprocessing_op_or_punc{"#"}));
    assert(try_consume(line, identifier{"define"}));
    skip_whitespace(line);
    assert(std::holds_alternative<identifier>(line.front().payload));
    std::string name = std::get<identifier>(line.front().payload).text;
    line = line.subspan(1);
    if (auto ptr = std::get_if<preprocessing_op_or_punc>(&line.front().payload); ptr && ptr->kind == "(") {
      assert(false);
      // TODO: function macro
    } else {
      // object macro
      assert(is_whitespace(line.front()));
      skip_whitespace(line);
      object_macros[name] =
        object_macro{name, std::vector<pp_token>(std::from_range, line.subspan(0, line.size() - 1))};
    }
  }

  void preprocess_line(std::span<const pp_token> line);

  std::vector<pp_token> preprocess(const std::vector<pp_token>& tokens) {
    if (tokens.empty()) return;
    assert(std::holds_alternative<newline>(tokens.back().payload));
  }
};

} // namespace ivl
