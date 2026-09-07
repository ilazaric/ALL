#pragma once

#include "../basic_parser"
#include <string>
#include <string_view>

namespace ivl::parsing::make {
struct logical_parser {
  std::string logical_contents;
  basic_parser logical;
  basic_parser physical;
  bool escaped = false;

  struct checkpoint {
    basic_parser::checkpoint logical;
    basic_parser::checkpoint physical;
  };

  checkpoint get_checkpoint() const { return {logical.get_checkpoint(), physical.get_checkpoint()}; }

  void restore_from_checkpoint(checkpoint c) {
    logical.restore_from_checkpoint(c.logical);
    physical.restore_from_checkpoint(c.physical);
  }

  logical_parser(std::string_view contents) : logical(contents), physical(contents) {
    while (!contents.empty()) {
      if (contents.starts_with("\\\n")) {
        contents.remove_prefix(2);
        continue;
      }
      if (contents.starts_with("\\\\")) {
        contents.remove_prefix(2);
        logical_contents += "\\\\";
        continue;
      }
      logical_contents += contents[0];
      contents.remove_prefix(1);
    }
    logical = basic_parser(logical_contents);
  }

  bool finished() const { return logical.finished(); }

  decltype(auto) current_sv() const { return logical.current_sv(); }
  decltype(auto) current_c() const { return logical.current_c(); }
  decltype(auto) slice(size_t lo, size_t hi) const { return logical.slice(lo, hi); }

  decltype(auto) debug_context() const { return physical.debug_context(); }
  decltype(auto) debug_context_file(size_t count) const { return physical.debug_context_file(count); }

  void consume_c_nocheck() {
    if (escaped) {
      physical.consume_c_nocheck();
      logical.consume_c_nocheck();
      while (physical.consume_if("\\\n"));
      escaped = false;
      return;
    }
    if (current_sv().starts_with("\\\\")) {
      physical.consume_c_nocheck();
      logical.consume_c_nocheck();
      escaped = true;
      return;
    }
    physical.consume_c_nocheck();
    logical.consume_c_nocheck();
    while (physical.consume_if("\\\n"));
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

  size_t get_cursor() const { return logical.cursor; }
};
} // namespace ivl::parsing::make
