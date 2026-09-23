#pragma once

#include <ivl/format>
#include <ivl/utility>
#include <algorithm>
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

  std::string_view slice(size_t lo, size_t hi) const {
    contract_assert(lo <= hi);
    contract_assert(lo <= contents.size());
    contract_assert(hi <= contents.size());
    return contents.substr(0, hi).substr(lo);
  }

  std::string debug_context() const {
    if (finished()) {
      return ivl::fmt::format("row: {}, column: {}, at EOF", diag_row, diag_col);
    } else {
      return ivl::fmt::format("row: {}, column: {}, character {:?}", diag_row, diag_col, current_c());
    }
  }

  std::string debug_context_file(size_t count) const {
    std::vector<std::string_view> prev;
    std::vector<std::string_view> next;
    std::string_view line;
    // bool at_eof = finished();
    // bool at_newline = !finished() && current_c() == '\n';
    // bool file_ends_with_newline = contents.ends_with('\n');

    auto containing_line = [&](size_t idx) -> std::string_view {
      if (idx == contents.size()) {
        if (contents.empty() || contents.ends_with('\n')) return contents.substr(contents.size());
        --idx;
      }
      contract_assert(idx < contents.size());
      auto lo = idx, hi = idx + 1;
      while (lo && contents[lo - 1] != '\n') --lo;
      while (hi < contents.size() && contents[hi - 1] != '\n') ++hi;
      return contents.substr(0, hi).substr(lo);
    };

    line = containing_line(cursor);

    {
      auto curr = line.data();
      for (size_t i = 0; curr != contents.data() && i < count; ++i) {
        prev.push_back(containing_line(curr - contents.data() - 1));
        curr = prev.back().data();
      }
      std::ranges::reverse(prev);
    }

    {
      auto curr = line.data() + line.size();
      for (size_t i = 0; curr != contents.data() + contents.size() && i < count; ++i) {
        next.push_back(containing_line(curr - contents.data()));
        curr = next.back().data() + next.back().size();
      }
    }

    std::string ret;
    for (auto&& s : prev) ret += ivl::fmt::format(".  | {:?}\n", s);
    ret += ivl::fmt::format(">>>| {:?}\n", line);
    for (auto&& s : next) ret += ivl::fmt::format(".  | {:?}\n", s);
    return ret;
  }

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

  std::string_view line() const {
    auto loc = contents.find('\n');
    return loc == std::string_view::npos ? contents : contents.substr(0, loc + 1);
  }
};
} // namespace ivl::parsing
