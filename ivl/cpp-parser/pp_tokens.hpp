#pragma once

#include <ivl/cpp-parser/spliced_cxx_file>
#include <ivl/reflection/enumerators>
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
  std::string ud_suffix;

  static std::optional<raw_literal> try_parse(ivl::spliced_cxx_file::parsing_state& state);
};

struct preprocessing_op_or_punc {
  std::string kind;

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
        state.consume(op);
        return preprocessing_op_or_punc{std::string(op)};
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
  static std::optional<newline> try_parse(ivl::spliced_cxx_file::parsing_state& state) {
    if (!state.starts_with("\n")) return std::nullopt;
    state.consume("\n");
    return newline{};
  }
};

struct whitespace {
  std::string text;

  static std::optional<whitespace> try_parse(ivl::spliced_cxx_file::parsing_state& state) {
    auto pred = [](char c) { return c != '\n' && isspace(c); };

    if (!pred(state.front())) return std::nullopt;
    std::string ws;
    while (pred(state.front())) {
      ws += state.front();
      state.remove_prefix(1);
    }

    return whitespace{std::move(ws)};
  }
};

struct identifier {
  std::string text;
};

struct basic_c_char {
  char c;
};

struct escape_sequence {
  // TODO
};

struct universal_character_name {
  // TODO
};

struct c_char {
  std::variant<basic_c_char, escape_sequence, universal_character_name> payload;
};

struct character_literal {
  encoding_prefix ep;
  std::vector<c_char> c_char_seq;
};

struct pp_number {
  // TODO
};

struct header_name {
  // TODO
};

struct import_kw {
  // TODO
};

struct module_kw {
  // TODO
};

struct export_kw {
  // TODO
};

struct string_literal {
  // TODO
};

struct non_whitespace_garbage {
  // TODO
};

struct pp_token {
  std::variant<
    std::monostate, raw_literal, preprocessing_op_or_punc, single_line_comment, multi_line_comment, newline, whitespace,
    identifier, character_literal, pp_number, header_name, import_kw, module_kw, export_kw, string_literal,
    non_whitespace_garbage>
    payload;

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
  std::string ret;
  while (is_nondigit(state.front()) || is_digit(state.front())) {
    ret += state.front();
    state.remove_prefix(1);
  }
  if (worded_op_or_puncs.contains(ret)) return pp_token{preprocessing_op_or_punc{ret}};
  return pp_token{identifier{ret}};
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
        "\x09"
        "\x0B"
        "\x0C"
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
  std::string ud_suffix;
  if (ud_suffix_maybe) {
    if (std::get_if<preprocessing_op_or_punc>(&ud_suffix_maybe->payload)) state = saved_state;
    else ud_suffix = std::get<identifier>(ud_suffix_maybe->payload).text;
  }

  return raw_literal{ep, delimiter, content, ud_suffix};
}
} // namespace ivl
