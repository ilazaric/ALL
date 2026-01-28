#pragma once

#include <ivl/cpp-parser/spliced_cxx_file>
#include <ivl/logger>
#include <ivl/meta>
#include <ivl/reflection/enumerators>
#include <ivl/reflection/test_attribute>
#include <ivl/util>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <format>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

namespace ivl {
enum class encoding_prefix : char {
  NONE,
  u8,
  u,
  U,
  L,
};

std::string_view encoding_prefix_str(encoding_prefix e) {
  switch (e) {
    using enum encoding_prefix;
  case NONE:
    return "";
  case u8:
    return "u8";
  case u:
    return "u";
  case U:
    return "U";
  case L:
    return "L";
  }
  throw std::runtime_error(std::format("Unknown encoding_prefix: {}", (int)std::to_underlying(e)));
}

struct raw_literal {
  encoding_prefix ep;
  std::string_view delimiter;
  std::string_view payload;
  std::string_view ud_suffix;

  static std::optional<raw_literal> try_parse(ivl::spliced_cxx_file::parsing_state& state);

  auto operator<=>(const raw_literal&) const = default;
};

struct preprocessing_op_or_punc {
  std::string_view kind;

  auto operator<=>(const preprocessing_op_or_punc&) const = default;

  static std::optional<preprocessing_op_or_punc> try_parse_symbolic(ivl::spliced_cxx_file::parsing_state& state) {
    static const auto regular_op_or_puncs = [] {
      std::vector<std::string_view> regular_op_or_puncs{
        "#", "##", "%:",  "%:%:", "{",  "}",  "[",  "]",  "(",   ")",   "[:", ":]", "<%", "%>", "<:", ":>",
        ";", ":",  "...", "?",    "::", ".",  ".*", "->", "->*", "^^",  "~",  "!",  "+",  "-",  "*",  "/",
        "%", "^",  "&",   "|",    "=",  "+=", "-=", "*=", "/=",  "%=",  "^=", "&=", "|=", "==", "!=", "<",
        ">", "<=", ">=",  "<=>",  "&&", "||", "<<", ">>", "<<=", ">>=", "++", "--", ",",
      };
      std::ranges::sort(regular_op_or_puncs, std::ranges::greater{}, [](std::string_view sv) { return sv.size(); });
      return regular_op_or_puncs;
    }();
    for (auto op : regular_op_or_puncs) {
      if (state.starts_with(op)) {
        auto start = state.begin();
        state.consume(op);
        return preprocessing_op_or_punc{std::string_view(start, state.begin())};
      }
    }
    // worded handled with identifier
    return std::nullopt;
  };

  static std::optional<preprocessing_op_or_punc>
  try_parse_digraph_exception_1(ivl::spliced_cxx_file::parsing_state& state) {
    if (!state.starts_with("<::")) return std::nullopt;
    if (state.starts_with("<:::") || state.starts_with("<::>")) return std::nullopt;
    state.consume('<');
    return preprocessing_op_or_punc{"<"};
  }

  static std::optional<preprocessing_op_or_punc>
  try_parse_digraph_exception_2(ivl::spliced_cxx_file::parsing_state& state) {
    if (!state.starts_with("[::") && !state.starts_with("[:>")) return std::nullopt;
    if (state.starts_with("[:::")) return std::nullopt;
    state.consume('[');
    return preprocessing_op_or_punc{"["};
  }
};

struct single_line_comment {
  std::string_view text;

  auto operator<=>(const single_line_comment&) const = default;

  static std::optional<single_line_comment> try_parse(ivl::spliced_cxx_file::parsing_state& state) {
    if (!state.starts_with("//")) return std::nullopt;
    auto start_ptr = state.begin();
    auto end = state.remaining.find('\n');
    if (end == std::string_view::npos)
      throw std::runtime_error(
        std::format("ICE? single line comment cannot find newline\n{}", state.debug_context(start_ptr))
      );
    state.remove_prefix(end);
    return single_line_comment{std::string_view{start_ptr, end}};
  }
};

struct multi_line_comment {
  std::string_view text;

  auto operator<=>(const multi_line_comment&) const = default;

  static std::optional<multi_line_comment> try_parse(ivl::spliced_cxx_file::parsing_state& state) {
    if (!state.starts_with("/*")) return std::nullopt;
    auto start_ptr = state.begin();
    state.consume("/*");
    auto end = state.remaining.find("*/");
    if (end == std::string_view::npos)
      throw std::runtime_error(std::format("Incomplete multiline comment\n{}", state.debug_context(start_ptr)));
    auto end_ptr = state.begin() + end + 2;
    state.remove_prefix(end + 2);
    return multi_line_comment{std::string_view{start_ptr, end_ptr}};
  }
};

struct newline {
  auto operator<=>(const newline&) const = default;
  static std::optional<newline> try_parse(ivl::spliced_cxx_file::parsing_state& state) {
    if (!state.starts_with("\n")) return std::nullopt;
    state.consume("\n");
    return newline{};
  }
};

struct whitespace {
  std::string_view text;

  auto operator<=>(const whitespace&) const = default;

  static std::optional<whitespace> try_parse(ivl::spliced_cxx_file::parsing_state& state) {
    auto pred = [](char c) { return c != '\n' && isspace(c); };

    if (!pred(state.front())) return std::nullopt;
    auto start = state.begin();
    while (pred(state.front())) state.remove_prefix(1);

    return whitespace{std::string_view(start, state.begin())};
  }
};

struct identifier {
  std::string_view text;
  auto operator<=>(const identifier&) const = default;
};

struct pp_number {
  std::string_view text;
  auto operator<=>(const pp_number&) const = default;
  static std::optional<pp_number> try_parse(spliced_cxx_file::parsing_state& state) {
    auto digit = [](char c) { return '0' <= c && c <= '9'; };
    auto nondigit = [](char c) { return 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_'; };
    auto sign = [](char c) { return c == '+' || c == '-'; };
    char c = state.starts_with('.') ? state.begin()[1] : state.front();
    if (!digit(c)) return std::nullopt;
    auto start = state.begin();
    if (state.starts_with('.')) state.consume('.');
    state.consume(c);

    while (true) {
      if (state.starts_with('.')) {
        state.consume('.');
        continue;
      }
      if (std::string_view("eEpP").contains(state.front()) && sign(state.begin()[1])) {
        state.consume(state.front());
        state.consume(state.front());
        continue;
      }
      if (state.starts_with('\'') && (digit(state.begin()[1]) || nondigit(state.begin()[1]))) {
        state.consume('\'');
        state.consume(state.front());
        continue;
      }
      if (digit(state.front()) || nondigit(state.front())) {
        state.consume(state.front());
        continue;
      }
      break;
    }

    auto end = state.begin();
    return pp_number{std::string_view(start, end)};
  }
};

struct basic_c_char {
  char c;
  auto operator<=>(const basic_c_char&) const = default;
  static std::optional<basic_c_char> try_parse(spliced_cxx_file::parsing_state& state) {
    auto c = state.front();
    if (c == '\'' || c == '\\' || c == '\n') return std::nullopt;
    state.consume(c);
    return basic_c_char{c};
  }
};

struct simple_escape_sequence {
  char c;
  auto operator<=>(const simple_escape_sequence&) const = default;
  static std::optional<simple_escape_sequence> try_parse(spliced_cxx_file::parsing_state& state) {
    if (!state.starts_with('\\')) return std::nullopt;
    auto copy = state;
    copy.consume('\\');
    if (!std::string_view("'\"?\\abfnrtv").contains(copy.front())) return std::nullopt;
    state = copy;
    char c = state.front();
    state.consume(c);
    return simple_escape_sequence{c};
  }
};

struct c_char {
  // TODO: all the rest
  std::variant<basic_c_char, simple_escape_sequence> payload;
  auto operator<=>(const c_char&) const = default;
  c_char(meta::same_as_one_of<basic_c_char, simple_escape_sequence> auto arg) : payload(arg) {}
  static std::optional<c_char> try_parse(spliced_cxx_file::parsing_state& state) {
    std::optional<c_char> ret;
    if (!ret) ret = basic_c_char::try_parse(state);
    if (!ret) ret = simple_escape_sequence::try_parse(state);
    return ret;
  }
};

struct character_literal {
  encoding_prefix ep;
  std::vector<c_char> c_char_seq;
  std::string_view ud_suffix;
  auto operator<=>(const character_literal&) const = default;

  static std::optional<character_literal> try_parse(spliced_cxx_file::parsing_state& state);
};

struct basic_s_char {
  char c;
  auto operator<=>(const basic_s_char&) const = default;
  static std::optional<basic_s_char> try_parse(spliced_cxx_file::parsing_state& state) {
    auto c = state.front();
    if (c == '"' || c == '\\' || c == '\n') return std::nullopt;
    state.consume(c);
    return basic_s_char{c};
  }
};

struct hexadecimal_escape_sequence {
  std::string_view text;
  auto operator<=>(const hexadecimal_escape_sequence&) const = default;
  static std::optional<hexadecimal_escape_sequence> try_parse(spliced_cxx_file::parsing_state& state) {
    if (!state.starts_with("\\x")) return std::nullopt;
    auto start = state.begin();
    auto saved_state = state;
    state.consume("\\x");
    bool curly = state.starts_with('{');
    if (curly) state.consume('{');
    while ('0' <= state.front() && state.front() <= '9' || 'a' <= state.front() && state.front() <= 'f' ||
           'A' <= state.front() && state.front() <= 'F')
      state.consume(state.front());
    if (curly) {
      if (!state.starts_with('}')) {
        state = saved_state;
        return std::nullopt;
      }
      state.consume('}');
    }
    return hexadecimal_escape_sequence{std::string_view{start, state.begin()}};
  }
};

struct s_char {
  // TODO: all the rest
  std::variant<basic_s_char, simple_escape_sequence, hexadecimal_escape_sequence> payload;
  auto operator<=>(const s_char&) const = default;
  s_char(meta::same_as_one_of<basic_s_char, simple_escape_sequence, hexadecimal_escape_sequence> auto arg)
      : payload(arg) {}
  static std::optional<s_char> try_parse(spliced_cxx_file::parsing_state& state) {
    std::optional<s_char> ret;
    if (!ret) ret = basic_s_char::try_parse(state);
    if (!ret) ret = simple_escape_sequence::try_parse(state);
    if (!ret) ret = hexadecimal_escape_sequence::try_parse(state);
    return ret;
  }
};

struct string_literal {
  encoding_prefix ep;
  std::vector<s_char> s_char_seq;
  std::string_view ud_suffix;
  auto operator<=>(const string_literal&) const = default;
  static std::optional<string_literal> try_parse(spliced_cxx_file::parsing_state& state);
};

struct header_name {
  std::string_view text;
  auto operator<=>(const header_name&) const = default;
  static std::optional<header_name> try_parse(spliced_cxx_file::parsing_state& state) {
    if (state.front() != '"' && state.front() != '<') return std::nullopt;
    char start_c = state.front();
    char end_c = start_c == '"' ? '"' : '>';
    auto start = state.begin();
    auto saved_state = state;
    state.consume(start_c);
    while (!state.starts_with(end_c)) {
      if (state.starts_with('\n')) {
        state = saved_state;
        return std::nullopt;
      }
      state.consume(state.front());
    }
    state.consume(end_c);
    return header_name{std::string_view{start, state.begin()}};
  }
};

struct non_whitespace_garbage {
  // TODO
  auto operator<=>(const non_whitespace_garbage&) const = default;
};

// gets synthesized in phase 4
struct placemarker {
  auto operator<=>(const non_whitespace_garbage&) const = default;
};

// module, import, export keywords not implemented (treated as regular identifier)
struct pp_token {
  std::variant<
    std::monostate, raw_literal, preprocessing_op_or_punc, single_line_comment, multi_line_comment, newline, whitespace,
    identifier, character_literal, pp_number, header_name, string_literal, non_whitespace_garbage, placemarker>
    payload;
  auto operator<=>(const pp_token&) const = default;

  pp_token(auto&& arg)
    requires(!std::same_as<pp_token, std::decay_t<decltype(arg)>>) && requires { payload = FWD(arg); }
      : payload(FWD(arg)) {}
};

std::optional<pp_token> try_parse_identifier_or_worded_op_or_punc(ivl::spliced_cxx_file::parsing_state& state) {
  static const std::set<std::string_view> worded_op_or_puncs{
    "and", "or", "xor", "not", "bitand", "bitor", "compl", "and_eq", "or_eq", "xor_eq", "not_eq",
  };
  auto is_nondigit = [](char c) { return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_'; };
  auto is_digit = [](char c) { return c >= '0' && c <= '9'; };
  if (!is_nondigit(state.front())) return std::nullopt;
  auto start = state.begin();
  while (is_nondigit(state.front()) || is_digit(state.front())) state.remove_prefix(1);
  std::string_view ret(start, state.begin());
  if (worded_op_or_puncs.contains(ret)) return pp_token{preprocessing_op_or_punc{ret}};
  return pp_token{identifier{ret}};
}

std::optional<character_literal> character_literal::try_parse(spliced_cxx_file::parsing_state& state) {
  encoding_prefix ep;
  bool parses_as = false;
  for (auto e : ivl::enumerators<encoding_prefix>()) {
    if (!state.starts_with(encoding_prefix_str(e))) continue;
    auto copy = state;
    copy.consume(encoding_prefix_str(e));
    if (!copy.starts_with('\'')) continue;
    ep = e;
    parses_as = true;
    break;
  }
  if (!parses_as) return std::nullopt;
  // Not specified this way, but assuming that ' must mean a character literal.
  // TODO: find out if this is an issue

  state.consume(encoding_prefix_str(ep));
  state.consume('\'');

  std::vector<c_char> c_char_seq;
  while (!state.starts_with('\'')) {
    auto cc = c_char::try_parse(state);
    if (!cc) throw std::runtime_error(std::format("Failed to parse c-char\n{}", state.debug_context()));
    c_char_seq.push_back(*cc);
  }

  state.consume('\'');

  auto saved_state = state;
  auto ud_suffix_maybe = try_parse_identifier_or_worded_op_or_punc(state);
  std::string_view ud_suffix;
  if (ud_suffix_maybe) {
    if (std::get_if<preprocessing_op_or_punc>(&ud_suffix_maybe->payload)) state = saved_state;
    else ud_suffix = std::get<identifier>(ud_suffix_maybe->payload).text;
  }

  return character_literal{ep, std::move(c_char_seq), ud_suffix};
}

std::optional<string_literal> string_literal::try_parse(spliced_cxx_file::parsing_state& state) {
  encoding_prefix ep;
  bool parses_as = false;
  for (auto e : ivl::enumerators<encoding_prefix>()) {
    if (!state.starts_with(encoding_prefix_str(e))) continue;
    auto copy = state;
    copy.consume(encoding_prefix_str(e));
    if (!copy.starts_with('"')) continue;
    ep = e;
    parses_as = true;
    break;
  }
  if (!parses_as) return std::nullopt;
  // Not specified this way, but assuming that " must mean a string literal.
  // TODO: find out if this is an issue

  state.consume(encoding_prefix_str(ep));
  state.consume('"');

  std::vector<s_char> s_char_seq;
  while (!state.starts_with('"')) {
    auto sc = s_char::try_parse(state);
    if (!sc) throw std::runtime_error(std::format("Failed to parse s-char\n{}", state.debug_context()));
    s_char_seq.push_back(*sc);
  }

  state.consume('"');

  auto saved_state = state;
  auto ud_suffix_maybe = try_parse_identifier_or_worded_op_or_punc(state);
  std::string_view ud_suffix;
  if (ud_suffix_maybe) {
    if (std::get_if<preprocessing_op_or_punc>(&ud_suffix_maybe->payload)) state = saved_state;
    else ud_suffix = std::get<identifier>(ud_suffix_maybe->payload).text;
  }

  return string_literal{ep, std::move(s_char_seq), ud_suffix};
}

std::optional<raw_literal> raw_literal::try_parse(ivl::spliced_cxx_file::parsing_state& state) {
  const auto& file = *state.file;
  encoding_prefix ep;
  bool parses_as_raw_literal = false;
  for (auto e : ivl::enumerators<encoding_prefix>()) {
    if (!state.starts_with(encoding_prefix_str(e))) continue;
    auto copy = state;
    copy.consume(encoding_prefix_str(e));
    if (!copy.starts_with("R\"")) continue;
    ep = e;
    parses_as_raw_literal = true;
    break;
  }
  if (!parses_as_raw_literal) return std::nullopt;

  state.consume(encoding_prefix_str(ep));
  state.consume("R");
  // pointing at `"`, not consuming it because we are in "phase-2 revert" land
  assert(state.front() == '"');
  auto delimiter_start = file.convert(ivl::spliced_cxx_file::splice_ptr{state.begin()}) + 1;
  auto delimiter_start_pos = delimiter_start - file.origin_begin();
  auto delimiter_end_pos = file.original_contents.find('(', delimiter_start_pos);
  if (delimiter_end_pos == std::string_view::npos) {
    throw std::runtime_error(
      std::format(
        "Malformed raw string literal, cannot deduce delimiter because opening paren `(` is missing\n{}",
        state.debug_context()
      )
    );
  }

  auto delimiter = std::string_view(file.original_contents).substr(0, delimiter_end_pos).substr(delimiter_start_pos);
  // https://eel.is/c++draft/lex#nt:d-char
  if (auto bad_char_pos = delimiter.find_first_of(
        " ()\\"
        "\x{09}"
        "\x{0B}"
        "\x{0C}"
        "\n"
      );
      bad_char_pos != std::string_view::npos) {
    throw std::runtime_error(
      std::format(
        "Malformed raw string literal, delimiter contains illegal character [{}]\n{}", (int)delimiter[bad_char_pos],
        state.debug_context()
      )
    );
  }

  auto content_start_pos = delimiter_end_pos + 1;
  auto content_end_pos = file.original_contents.find(std::format("){}\"", delimiter), content_start_pos);
  if (content_end_pos == std::string_view::npos) {
    throw std::runtime_error(
      std::format(
        "Malformed raw string literal, cannot deduce end because ending delimiter `){}\"` is  missing\n{}", delimiter,
        state.debug_context()
      )
    );
  }

  auto content = std::string_view(file.original_contents).substr(0, content_end_pos).substr(content_start_pos);
  auto end_quote_ptr = content.data() + content.size() + 1 + delimiter.size();
  state.remaining = std::string_view(file.post_splicing_contents)
                      .substr(file.convert(ivl::spliced_cxx_file::origin_ptr{end_quote_ptr}) - file.splice_begin());
  state.consume('"');

  auto saved_state = state;
  auto ud_suffix_maybe = try_parse_identifier_or_worded_op_or_punc(state);
  std::string_view ud_suffix;
  if (ud_suffix_maybe) {
    if (std::get_if<preprocessing_op_or_punc>(&ud_suffix_maybe->payload)) state = saved_state;
    else ud_suffix = std::get<identifier>(ud_suffix_maybe->payload).text;
  }

  return raw_literal{ep, delimiter, content, ud_suffix};
}

std::vector<pp_token> top_level_parse(spliced_cxx_file::parsing_state& state) {
  std::vector<pp_token> tokens;

  auto tokens_ends_with = [&](const std::vector<pp_token>& seq) {
    if (tokens.empty()) return false;
    auto it = tokens.end();
    --it;

    for (size_t idx = seq.size() - 1; idx + 1; --idx) {
      auto&& target = seq[idx];

      while (std::holds_alternative<whitespace>(it->payload) ||
             std::holds_alternative<single_line_comment>(it->payload) ||
             std::holds_alternative<multi_line_comment>(it->payload)) {
        if (it == tokens.begin()) return idx == 0 && std::holds_alternative<newline>(target.payload);
        --it;
      }

      if (*it != target) return false;
      if (idx != 0) {
        if (it == tokens.begin()) return false;
        --it;
      }
    }

    return true;
  };

  // https://eel.is/c++draft/lex#pptoken-5.4.2
  std::vector<std::vector<pp_token>> header_name_contexts{
    {
      newline{},
      preprocessing_op_or_punc{"#"},
      identifier{"include"},
    },
    {
      newline{},
      preprocessing_op_or_punc{"#"},
      identifier{"embed"},
    },
    {
      newline{},
      identifier{"import"},
    },
    {
      newline{},
      identifier{"export"},
      identifier{"import"},
    },
  };
  std::vector<std::vector<pp_token>> header_name_conditional_contexts{
    {identifier{"__has_include"}, preprocessing_op_or_punc{"("}},
    {identifier{"__has_embed"}, preprocessing_op_or_punc{"("}},
  };

  enum class conditional {
    START_OF_LINE,
    HASH,
    CORRECT,
    FAILED,
  };
  conditional current_conditional = conditional::START_OF_LINE;
  auto handle_conditional = [&](const pp_token& token) {
    if (std::holds_alternative<newline>(token.payload)) {
      current_conditional = conditional::START_OF_LINE;
      return;
    }
    if (std::holds_alternative<whitespace>(token.payload) ||
        std::holds_alternative<single_line_comment>(token.payload) ||
        std::holds_alternative<multi_line_comment>(token.payload) || current_conditional == conditional::FAILED ||
        current_conditional == conditional::CORRECT)
      return;
    if (current_conditional == conditional::START_OF_LINE) {
      if (auto ptr = std::get_if<preprocessing_op_or_punc>(&token.payload); ptr && ptr->kind == "#")
        current_conditional = conditional::HASH;
      else current_conditional = conditional::FAILED;
      return;
    }
    assert(current_conditional == conditional::HASH);
    if (auto ptr = std::get_if<identifier>(&token.payload);
        ptr && (ptr->text == "if" || ptr->text == "elif" || ptr->text == "embed"))
      current_conditional = conditional::CORRECT;
    else current_conditional = conditional::FAILED;
  };

  auto should_try_header_name = [&] {
    for (auto&& context : header_name_contexts)
      if (tokens_ends_with(context)) return true;
    if (current_conditional != conditional::CORRECT) return false;
    for (auto&& context : header_name_conditional_contexts)
      if (tokens_ends_with(context)) return true;
    return false;
  };

  while (!state.empty()) {
    auto copy_state = state;
    std::optional<pp_token> parsed;

    if (should_try_header_name()) parsed = header_name::try_parse(state);

    if (!parsed) parsed = single_line_comment::try_parse(state);
    if (!parsed) parsed = multi_line_comment::try_parse(state);

    if (!parsed) parsed = whitespace::try_parse(state);
    if (!parsed) parsed = newline::try_parse(state);

    if (!parsed) parsed = raw_literal::try_parse(state);
    if (!parsed) parsed = preprocessing_op_or_punc::try_parse_digraph_exception_1(state);
    if (!parsed) parsed = preprocessing_op_or_punc::try_parse_digraph_exception_2(state);

    if (!parsed) parsed = pp_number::try_parse(state);

    if (!parsed) parsed = preprocessing_op_or_punc::try_parse_symbolic(state);

    if (!parsed) parsed = character_literal::try_parse(state);
    if (!parsed) parsed = string_literal::try_parse(state);

    if (!parsed) parsed = try_parse_identifier_or_worded_op_or_punc(state);

    if (!parsed) {
      throw std::runtime_error(std::format("ICE: parsing failed\n{}", state.debug_context()));
    }

    // a big like this can blow up memory
    assert(copy_state.begin() != state.begin());

    handle_conditional(*parsed);

    tokens.push_back(std::move(*parsed));
  }

  return tokens;
}

std::string reserialize(const pp_token& token) {
  return token.payload.visit([&]<typename T>(const T& unpacked) -> std::string {
    if constexpr (std::same_as<T, newline>) {
      return "\n";
    } else if constexpr (std::same_as<T, whitespace>) {
      return (std::string)unpacked.text;
    } else if constexpr (std::same_as<T, identifier>) {
      return (std::string)unpacked.text;
    } else if constexpr (std::same_as<T, raw_literal>) {
      return std::format(
        "{}R\"{}({}){}\"{}", encoding_prefix_str(unpacked.ep), unpacked.delimiter, unpacked.payload, unpacked.delimiter,
        unpacked.ud_suffix
      );
    } else if constexpr (std::same_as<T, preprocessing_op_or_punc>) {
      return (std::string)unpacked.kind;
    } else if constexpr (std::same_as<T, single_line_comment>) {
      return (std::string)unpacked.text;
    } else if constexpr (std::same_as<T, multi_line_comment>) {
      return (std::string)unpacked.text;
    } else if constexpr (std::same_as<T, pp_number>) {
      return (std::string)unpacked.text;
    } else if constexpr (std::same_as<T, character_literal>) {
      std::string ret{encoding_prefix_str(unpacked.ep)};
      ret += '\'';
      for (auto&& c : unpacked.c_char_seq) {
        if (auto ptr = std::get_if<basic_c_char>(&c.payload)) ret += ptr->c;
        else if (auto ptr = std::get_if<simple_escape_sequence>(&c.payload)) {
          ret += '\\';
          ret += ptr->c;
        } else assert(false);
      }
      ret += '\'';
      ret += unpacked.ud_suffix;
      return ret;
    } else if constexpr (std::same_as<T, string_literal>) {
      std::string ret{encoding_prefix_str(unpacked.ep)};
      ret += '"';
      for (auto&& c : unpacked.s_char_seq) {
        if (auto ptr = std::get_if<basic_s_char>(&c.payload)) ret += ptr->c;
        else if (auto ptr = std::get_if<simple_escape_sequence>(&c.payload)) {
          ret += '\\';
          ret += ptr->c;
        } else if (auto ptr = std::get_if<hexadecimal_escape_sequence>(&c.payload)) {
          ret += ptr->text;
        } else assert(false);
      }
      ret += '"';
      ret += unpacked.ud_suffix;
      return ret;
    } else if constexpr (std::same_as<T, header_name>) {
      return (std::string)unpacked.text;
    } else {
      assert(false);
    }
  });
}

std::string test_token_kind(const pp_token& token) {
  return token.payload.visit([]<typename T>(const T&) { return std::string(util::typestr<T>()); });
}

// IVL has_test_variant()
[[= test]] void test_raw_literal() {
  spliced_cxx_file file("u8R\"delim(con\ntent)delim\"sv");
  auto state = file.parsing_start();
  auto tokens = top_level_parse(state);
  assert(tokens.size() == 2);
  auto raw = std::get<raw_literal>(tokens[0].payload);
  assert(raw.delimiter == "delim");
  assert(raw.ep == encoding_prefix::u8);
  assert(raw.payload == "con\ntent");
  assert(raw.ud_suffix == "sv");
  std::get<newline>(tokens[1].payload);
}

[[= test]] void test_identifier() {
  spliced_cxx_file file("a\\  \nba\\  \nba\\  \nb");
  auto state = file.parsing_start();
  auto tokens = top_level_parse(state);
  assert(tokens.size() == 2);
  auto id = std::get<identifier>(tokens[0].payload);
  assert(id.text == "ababab");
  std::get<newline>(tokens[1].payload);
}

[[= test]] void test_character_literal() {
  spliced_cxx_file file("U'a\\nb'sv");
  auto state = file.parsing_start();
  auto tokens = top_level_parse(state);
  assert(tokens.size() == 2);
  auto cl = std::get<character_literal>(tokens[0].payload);
  assert(cl.ep == encoding_prefix::U);
  assert(cl.c_char_seq.size() == 3);
  assert(std::get<basic_c_char>(cl.c_char_seq[0].payload).c == 'a');
  assert(std::get<simple_escape_sequence>(cl.c_char_seq[1].payload).c == 'n');
  assert(std::get<basic_c_char>(cl.c_char_seq[2].payload).c == 'b');
  assert(cl.ud_suffix == "sv");
}

[[= test]] void test_string_literal() {
  spliced_cxx_file file("L\"a\\nb\"svsvsv");
  auto state = file.parsing_start();
  auto tokens = top_level_parse(state);
  assert(tokens.size() == 2);
  auto cl = std::get<string_literal>(tokens[0].payload);
  assert(cl.ep == encoding_prefix::L);
  assert(cl.s_char_seq.size() == 3);
  assert(std::get<basic_s_char>(cl.s_char_seq[0].payload).c == 'a');
  assert(std::get<simple_escape_sequence>(cl.s_char_seq[1].payload).c == 'n');
  assert(std::get<basic_s_char>(cl.s_char_seq[2].payload).c == 'b');
  assert(cl.ud_suffix == "svsvsv");
}

[[= test]] void test_pp_number() {
  spliced_cxx_file file(".1abcDEF456'1'Xe+E-p+P-....");
  auto state = file.parsing_start();
  auto tokens = top_level_parse(state);
  assert(tokens.size() == 2);
  auto num = std::get<pp_number>(tokens[0].payload);
  assert(num.text == ".1abcDEF456'1'Xe+E-p+P-....");
}

[[= test]] void test_header_name() {
  spliced_cxx_file file("  /**/ # /**/ include /**/ <header_name> /**/");
  auto state = file.parsing_start();
  auto tokens = top_level_parse(state);
  assert(tokens.size() == 15);
  auto hdr = std::get<header_name>(tokens[11].payload);
  assert(hdr.text == "<header_name>");
}
} // namespace ivl
