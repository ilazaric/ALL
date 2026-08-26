#pragma once

#include "../parser_declaration"
#include "../parser_one"
#include "../raw_arguments"
#include <charconv>
#include <concepts>
#include <print>
#include <string_view>

namespace ivl::cmdline_parsing {
template<std::integral Ip>
struct parser<Ip> : parser_one {
  bool parse_one(Ip& arg, std::string_view sv) const {
    auto ret = std::from_chars(sv.data(), sv.data() + sv.size(), arg);
    if (ret && ret.ptr == sv.data() + sv.size()) return true;
    else {
      println("failed to parse integer, argument: {:?}", sv);
      return false;
    }
  }
};
} // namespace ivl::cmdline_parsing
