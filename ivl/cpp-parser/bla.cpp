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

std::vector<std::string_view> physical_lines(std::string_view data) {
  if (data.empty()) return {};
  std::vector<std::string_view> lines;
  std::string_view rem = data;
  while (true) {
    auto loc = rem.find('\n');
    lines.push_back(rem.substr(0, loc));
    if (loc == std::string_view::npos) break;
    rem.remove_prefix(loc + 1);
  }
  return lines;
}

std::vector<std::vector<std::string_view>> logical_lines(const std::vector<std::string_view>& lines) {
  if (lines.empty()) return {};
  std::vector<std::vector<std::string_view>> logical_lines;
  {
    auto backslash = lines.back().rfind('\\');
    if (backslash != std::string_view::npos && std::ranges::all_of(lines.back().substr(backslash + 1), &isspace))
      throw std::runtime_error("Last line ends with backslash `\\`");
  }
  logical_lines.emplace_back();
  for (auto line : lines) {
    auto backslash = line.rfind('\\');
    if (backslash != std::string_view::npos && std::ranges::all_of(line.substr(backslash + 1), &isspace)) {
      logical_lines.back().push_back(line.substr(0, backslash));
    } else {
      logical_lines.back().push_back(line);
      logical_lines.emplace_back();
    }
  }
  if (lines.back().empty()) logical_lines.pop_back();
  // assert(!logical_lines.empty() && logical_lines.back().empty());
  return logical_lines;
}

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
};

struct multi_line_comment {
  std::string_view text;
};

struct newline {};

struct whitespace {
  std::string text;
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
};

int main(int argc, char* argv[]) {
  assert(argc == 2);
  auto data_storage = ivl::linux::read_file(argv[1]);
  // if (!data_storage.empty() && data_storage.back() != '\n') data_storage.push_back('\n');
  assert(data_storage.empty() || !data_storage.back() != '\\');
  std::string_view data = data_storage;

  std::vector<std::string_view> lines = physical_lines(data);

  for (auto&& line : lines) LOG(line);

  std::vector<std::vector<std::string_view>> lls = logical_lines(lines);

  for (auto&& ll : lls) {
    std::string line;
    for (auto x : ll) line += x;
    LOG(line);
  }

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

  size_t logic_line_idx = 0;
  size_t physical_line_idx = 0;
  size_t char_idx = 0;

  auto describe_c = [](char c) {
    // TODO: if c is printable, print it
    return std::format("[{}]", (int)c);
  };

  auto reached_eof = [&] {
    if (lls.empty()) return true;
    return logic_line_idx == lls.size() - 1;
  };

  auto current_c = [&] {
    if (logic_line_idx == lls.size()) throw std::runtime_error("ICE: Reached EOF and asked for current character");
    if (physical_line_idx == lls[logic_line_idx].size()) return '\n';
    return lls[logic_line_idx][physical_line_idx][char_idx];
  };

  auto current_addr = [&] {
    if (logic_line_idx == lls.size())
      throw std::runtime_error("ICE: Reached EOF and asked for address of current character");
    if (physical_line_idx == lls[logic_line_idx].size()) {
      // throw std::runtime_error("ICE: Reached end-of-line and asked for address of current character");
      assert(physical_line_idx != 0);
      auto ptr = lls[logic_line_idx][physical_line_idx - 1].data();
      while (ptr < data.data() + data.size() && *ptr != '\n') ++ptr;
      assert(ptr != data.data() + data.size());
      return ptr;
    }
    return &lls[logic_line_idx][physical_line_idx][char_idx];
  };

  auto debug_context_at = [&](const char* ptr) {
    assert(*ptr != '\n');
    size_t row = 0;
    while (row < lines.size() && lines[row].data() + lines[row].size() <= ptr) ++row;
    assert(row != lines.size());
    auto line = lines[row];
    size_t col = ptr - line.data();
    return std::format("{}\n{: <{}}^ here (row:{}, col:{})\n", line, "", col, row + 1, col + 1);
  };

  auto debug_context = [&] {
    if (reached_eof()) {
      return std::format("At EOF");
    } else if (current_c() == '\n') {
      assert(physical_line_idx != 0);
      return std::format("{}\nAt new-line at end\n", lls[logic_line_idx][physical_line_idx - 1]);
    } else return debug_context_at(current_addr());
  };

  auto consume_c = [&](char c) {
    if (current_c() != c)
      throw std::runtime_error(
        std::format(
          "ICE: Tried to consume different character; current={}, attempted={}\n", describe_c(current_c()),
          describe_c(c), debug_context()
        )
      );

    if (physical_line_idx == lls[logic_line_idx].size()) {
      ++logic_line_idx;
      physical_line_idx = 0;
      char_idx = 0;
      if (logic_line_idx == lls.size()) return;
      while (physical_line_idx < lls[logic_line_idx].size() && lls[logic_line_idx][physical_line_idx].empty())
        ++physical_line_idx;
      return;
    }

    ++char_idx;
    while (physical_line_idx < lls[logic_line_idx].size() &&
           char_idx == lls[logic_line_idx][physical_line_idx].size()) {
      ++physical_line_idx;
      char_idx = 0;
    }
  };

  auto consume = [&](std::string_view sv) {
    for (auto c : sv) consume_c(c);
  };

  auto save_state = [&] { return std::make_tuple(logic_line_idx, physical_line_idx, char_idx); };

  auto restore_state = [&](std::tuple<size_t, size_t, size_t> state) {
    std::tie(logic_line_idx, physical_line_idx, char_idx) = state;
  };

  auto starts_with = [&](auto&&... svs) {
    auto logic_line_idx_store = logic_line_idx;
    auto physical_line_idx_store = physical_line_idx;
    auto char_idx_store = char_idx;

    bool ans = true;
    try {
      (consume(svs), ...);
    } catch (const std::runtime_error&) {
      ans = false;
    }

    logic_line_idx = logic_line_idx_store;
    physical_line_idx = physical_line_idx_store;
    char_idx = char_idx_store;
    return ans;
  };

  std::set<std::string_view> worded_op_or_puncs{
    "and", "or", "xor", "not", "bitand", "bitor", "compl", "and_eq", "or_eq", "xor_eq", "not_eq",
  };

  auto try_parse_identifier_or_worded_op_or_punc = [&] -> std::optional<pp_token> {
    auto is_nondigit = [](char c) { return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_'; };
    auto is_digit = [](char c) { return c >= '0' && c <= '9'; };
    if (!is_nondigit(current_c())) return std::nullopt;
    std::string ret;
    while (is_nondigit(current_c()) || is_digit(current_c())) {
      ret += current_c();
      consume_c(current_c());
    }
    if (worded_op_or_puncs.contains(ret)) return pp_token{preprocessing_op_or_punc{ret}};
    return pp_token{identifier{ret}};
  };

  auto try_parse_raw_string_literal = [&] -> std::optional<pp_token> {
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

    consume(encoding_prefix_str(ep));
    consume("R");
    // pointing at `"`, not consuming it because we are in "phase-2 revert" land
    assert(current_c() == '"');
    auto delimiter_start = current_addr() + 1;
    auto delimiter_start_pos = delimiter_start - data.data();
    auto delimiter_end_pos = data.find('(', delimiter_start_pos);
    if (delimiter_end_pos == std::string_view::npos) {
      throw std::runtime_error(
        std::format(
          "Malformed raw string literal, cannot deduce delimiter because opening paren `(` is missing\n{}",
          debug_context_at(delimiter_start - 1)
        )
      );
    }

    auto delimiter = data.substr(0, delimiter_end_pos).substr(delimiter_start_pos);
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
          describe_c(delimiter[bad_char_pos]), debug_context_at(delimiter_start - 1)
        )
      );
    }

    auto content_start_pos = delimiter_end_pos + 1;
    auto content_end_pos = data.find(std::format("){}\"", delimiter), content_start_pos);
    if (content_end_pos == std::string_view::npos) {
      throw std::runtime_error(
        std::format(
          "Malformed raw string literal, cannot deduce end because ending delimiter `){}\"` is  missing\n{}", delimiter,
          debug_context_at(delimiter_start - 1)
        )
      );
    }

    auto content = data.substr(0, content_end_pos).substr(content_start_pos);
    auto end_quote_ptr = content.data() + content.size() + 1 + delimiter.size();
    while (current_addr() != end_quote_ptr) consume_c(current_c());
    consume_c('"');

    auto state = save_state();
    auto ud_suffix_maybe = try_parse_identifier_or_worded_op_or_punc();
    std::string ud_suffix;
    if (ud_suffix_maybe) {
      if (std::get_if<preprocessing_op_or_punc>(&ud_suffix_maybe->payload)) restore_state(state);
      else ud_suffix = std::get<identifier>(ud_suffix_maybe->payload).text;
    }

    return pp_token{raw_literal{ep, delimiter, content, ud_suffix}};
  };

  auto try_parse_digraph_exception_1 = [&] -> std::optional<pp_token> {
    if (!starts_with("<::")) return std::nullopt;
    if (starts_with("<:::") || starts_with("<::>")) return std::nullopt;
    consume_c('<');
    return pp_token{preprocessing_op_or_punc{"<"}};
  };

  auto try_parse_digraph_exception_2 = [&] -> std::optional<pp_token> {
    if (!starts_with("[::") && !starts_with("[:>")) return std::nullopt;
    if (starts_with("[:::")) return std::nullopt;
    consume_c('[');
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
        consume(op);
        return pp_token{preprocessing_op_or_punc{std::string(op)}};
      }
    }
    // worded handled with identifier
    return std::nullopt;
  };

  auto try_parse_single_line_comment = [&] -> std::optional<pp_token> {
    if (!starts_with("//")) return std::nullopt;
    auto start_ptr = current_addr();
    while (current_c() != '\n') consume_c(current_c());
    auto end_ptr = current_addr();
    return pp_token{single_line_comment{std::string_view{start_ptr, end_ptr}}};
  };

  auto try_parse_multi_line_comment = [&] -> std::optional<pp_token> {
    if (!starts_with("/*")) return std::nullopt;
    auto start_ptr = current_addr();
    consume("/*");
    while (!starts_with("*/")) consume_c(current_c());
    consume_c('*');
    auto end_ptr = current_addr() + 1;
    consume_c('/');
    return pp_token{multi_line_comment{std::string_view{start_ptr, end_ptr}}};
  };

  auto try_parse_whitespace = [&] -> std::optional<pp_token> {
    auto pred = [](char c) { return c != '\n' && isspace(c); };

    if (!pred(current_c())) return std::nullopt;
    std::string ws;
    while (pred(current_c())) {
      ws += current_c();
      consume_c(current_c());
    }

    return pp_token{whitespace{std::move(ws)}};
  };

  auto try_parse_newline = [&] -> std::optional<pp_token> {
    if (current_c() != '\n') return std::nullopt;
    consume_c('\n');
    return pp_token{newline{}};
  };

  LOG(lines.size(), lls.size());

  std::vector<pp_token> tokens;
  while (!reached_eof()) {
    LOG(logic_line_idx, physical_line_idx, char_idx, (int)current_c());

    std::optional<pp_token> parsed;

    if (!parsed) parsed = try_parse_single_line_comment();
    if (!parsed) parsed = try_parse_multi_line_comment();

    if (!parsed) parsed = try_parse_whitespace();
    if (!parsed) parsed = try_parse_newline();

    // TODO: spec is broken, add ud-suffix
    if (!parsed) parsed = try_parse_raw_string_literal();
    if (!parsed) parsed = try_parse_digraph_exception_1();
    if (!parsed) parsed = try_parse_digraph_exception_2();

    if (!parsed) parsed = try_parse_preprocessing_op_or_punc();

    if (!parsed) parsed = try_parse_identifier_or_worded_op_or_punc();

    if (!parsed) {
      throw std::runtime_error(std::format("ICE: parsing failed\n{}", debug_context()));
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
