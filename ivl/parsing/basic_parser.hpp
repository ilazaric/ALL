#pragma once

#include <ivl/utility>
#include <format>
#include <string_view>

namespace ivl::parsing {
struct basic_parser {
  std::string_view contents;
  size_t cursor = 0;
  size_t diag_row = 1;
  size_t diag_col = 1;

  explicit basic_parser(std::string_view contents) : contents(contents) {}

  basic_parser() = default;

  struct checkpoint {
    size_t cursor;
    size_t diag_row;
    size_t diag_col;
  };

  checkpoint get_checkpoint() const { return checkpoint{cursor, diag_row, diag_col}; }

  void restore_from_checkpoint(checkpoint c) {
    cursor = c.cursor;
    diag_row = c.diag_row;
    diag_col = c.diag_col;
  }

  std::string debug_context() const { return std::format("row: {}, column: {}", diag_row, diag_col); }

  bool finished() const { return cursor == contents.size(); }

  const char& current_c() const {
    finished() && panic("tried to peek character at EOF");
    return contents[cursor];
  }

  std::string_view current_sv() const { return contents.substr(cursor); }

  void consume_c_nocheck() {
    finished() && panic("tried to consume character at EOF");
    if (current_c() == '\n') {
      ++diag_row;
      diag_col = 1;
    } else ++diag_col;
    ++cursor;
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
};
} // namespace ivl::parsing
