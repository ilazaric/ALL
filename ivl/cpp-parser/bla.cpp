#include <ivl/cpp-parser/truc>
#include <ivl/linux/utils>
#include <ivl/logger>
#include <ivl/reflection/test_attribute>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <filesystem>
#include <functional>
#include <meta>
#include <set>
#include <variant>

enum class encoding_prefix : char {
  NONE,
  u8,
  u,
  U,
  L,
};

template <typename E>
  requires std::is_enum_v<E>
consteval std::span<const E> enumerators() {
  std::vector<E> ret;
  for (auto e : enumerators_of(^^E)) ret.push_back(extract<E>(e));
  return define_static_array(ret);
}

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
};

struct preprocessing_op_or_punc {
  std::string kind;
};

struct single_line_comment {
  std::string_view text;

  static std::optional<single_line_comment> try_parse(cxx_file::parsing_state& state) {
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

  static std::optional<multi_line_comment> try_parse(cxx_file::parsing_state& state) {
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
  static std::optional<newline> try_parse(cxx_file::parsing_state& state) {
    if (!state.starts_with("\n")) return std::nullopt;
    state.consume("\n");
    return newline{};
  }
};

struct whitespace {
  std::string text;

  static std::optional<whitespace> try_parse(cxx_file::parsing_state& state) {
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

int main(int argc, char* argv[]) {
  assert(argc == 2);

  cxx_file file(ivl::linux::read_file(argv[1]));
  LOG("\n" + file.post_splicing_contents);

  std::vector<std::filesystem::path> search_list{
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/../../../../include/c++/16.0.0",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/../../../../include/c++/16.0.0/x86_64-pc-linux-gnu",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/../../../../include/c++/16.0.0/backward",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/include",
    "/usr/local/include",
    "/opt/GCC-REFL/include",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/include-fixed",
    "/usr/include/x86_64-linux-gnu",
    "/usr/include",
  };

  auto state = file.parsing_start();

  auto describe_c = [](char c) {
    // TODO: if c is printable, print it
    return std::format("[{}]", (int)c);
  };

  auto starts_with = [&](auto&&... svs) {
    auto current = state;
    bool failed = false;
    (
      [&](std::string_view sv) {
        if (current.starts_with(sv)) current.remove_prefix(sv.size());
        else failed = true;
      }(svs),
      ...
    );
    return !failed;
  };

  std::set<std::string_view> worded_op_or_puncs{
    "and", "or", "xor", "not", "bitand", "bitor", "compl", "and_eq", "or_eq", "xor_eq", "not_eq",
  };

  auto try_parse_identifier_or_worded_op_or_punc = [&] -> std::optional<pp_token> {
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
  };

  auto try_parse_raw_string_literal = [&] -> std::optional<raw_literal> {
    encoding_prefix ep;
    bool parses_as_raw_literal = false;
    static constexpr auto eps = enumerators<encoding_prefix>();
    for (auto e : eps) {
      if (!starts_with(encoding_prefix_str(e), "R\"")) continue;
      ep = e;
      parses_as_raw_literal = true;
      break;
    }
    if (!parses_as_raw_literal) return std::nullopt;

    state.consume(encoding_prefix_str(ep));
    state.consume("R");
    // pointing at `"`, not consuming it because we are in "phase-2 revert" land
    assert(state.front() == '"');
    auto delimiter_start = file.convert(cxx_file::splice_ptr{state.begin()}) + 1;
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
          "Malformed raw string literal, delimiter contains illegal character `{}`\n{}",
          describe_c(delimiter[bad_char_pos]), state.debug_context()
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
                        .substr(file.convert(cxx_file::origin_ptr{end_quote_ptr}) - file.splice_begin());
    state.consume('"');

    auto saved_state = state;
    auto ud_suffix_maybe = try_parse_identifier_or_worded_op_or_punc();
    std::string ud_suffix;
    if (ud_suffix_maybe) {
      if (std::get_if<preprocessing_op_or_punc>(&ud_suffix_maybe->payload)) state = saved_state;
      else ud_suffix = std::get<identifier>(ud_suffix_maybe->payload).text;
    }

    return raw_literal{ep, delimiter, content, ud_suffix};
  };

  auto try_parse_digraph_exception_1 = [&] -> std::optional<pp_token> {
    if (!starts_with("<::")) return std::nullopt;
    if (starts_with("<:::") || starts_with("<::>")) return std::nullopt;
    state.consume('<');
    return pp_token{preprocessing_op_or_punc{"<"}};
  };

  auto try_parse_digraph_exception_2 = [&] -> std::optional<pp_token> {
    if (!starts_with("[::") && !starts_with("[:>")) return std::nullopt;
    if (starts_with("[:::")) return std::nullopt;
    state.consume('[');
    return pp_token{preprocessing_op_or_punc{"["}};
  };

  std::vector<std::string_view> regular_op_or_puncs{
    "#", "##", "%:",  "%:%:", "{",  "}",  "[",  "]",  "(",   ")",   "[:", ":]", "<%", "%>", "<:", ":>",
    ";", ":",  "...", "?",    "::", ".",  ".*", "->", "->*", "^^",  "~",  "!",  "+",  "-",  "*",  "/",
    "%", "^",  "&",   "|",    "=",  "+=", "-=", "*=", "/=",  "%=",  "^=", "&=", "|=", "==", "!=", "<",
    ">", "<=", ">=",  "<=>",  "&&", "||", "<<", ">>", "<<=", ">>=", "++", "--", ",",
  };

  std::ranges::sort(regular_op_or_puncs, std::ranges::greater{}, [](std::string_view sv) { return sv.size(); });
  // std::ranges::sort(worded_op_or_puncs, std::ranges::greater{}, [](std::string_view sv) { return sv.size(); });

  auto try_parse_preprocessing_op_or_punc = [&] -> std::optional<pp_token> {
    for (auto op : regular_op_or_puncs) {
      if (starts_with(op)) {
        state.consume(op);
        return pp_token{preprocessing_op_or_punc{std::string(op)}};
      }
    }
    // worded handled with identifier
    return std::nullopt;
  };

  std::vector<pp_token> tokens;
  while (!state.empty()) {
    std::optional<pp_token> parsed;

    if (!parsed) parsed = single_line_comment::try_parse(state);
    if (!parsed) parsed = multi_line_comment::try_parse(state);

    if (!parsed) parsed = whitespace::try_parse(state);
    if (!parsed) parsed = newline::try_parse(state);

    // TODO: spec is broken, add ud-suffix
    if (!parsed) parsed = try_parse_raw_string_literal();
    if (!parsed) parsed = try_parse_digraph_exception_1();
    if (!parsed) parsed = try_parse_digraph_exception_2();

    if (!parsed) parsed = try_parse_preprocessing_op_or_punc();

    if (!parsed) parsed = try_parse_identifier_or_worded_op_or_punc();

    if (!parsed) {
      throw std::runtime_error(std::format("ICE: parsing failed\n{}", state.debug_context()));
    }

    tokens.push_back(std::move(*parsed));
  }

  for (auto&& token : tokens) {
    token.payload.visit([&]<typename T>(const T& unpacked) {
      if constexpr (std::same_as<T, newline>) {
        std::cout << "\n";
      } else if constexpr (std::same_as<T, whitespace>) {
        std::cout << unpacked.text;
      } else if constexpr (std::same_as<T, identifier>) {
        std::cout << unpacked.text;
      } else if constexpr (std::same_as<T, raw_literal>) {
        std::cout << encoding_prefix_str(unpacked.ep) << "R\"" << unpacked.delimiter << "(" << unpacked.payload << ")"
                  << unpacked.delimiter << "\"" << unpacked.ud_suffix;
      } else if constexpr (std::same_as<T, preprocessing_op_or_punc>) {
        std::cout << unpacked.kind;
      } else if constexpr (std::same_as<T, single_line_comment>) {
        std::cout << unpacked.text;
      } else if constexpr (std::same_as<T, multi_line_comment>) {
        std::cout << unpacked.text;
      } else {
        assert(false);
      }
    });
  }
}
