#pragma once

#include <ivl/type_traits/same_as_one_of>
#include "../parser_declaration"
#include "../parser_one"
#include "../raw_arguments"
#include <filesystem>
#include <string>
#include <string_view>

namespace ivl::cmdline_parsing {
template<same_as_one_of<std::string_view, std::string, std::filesystem::path> Str>
struct parser<Str> : parser_one {
  inline bool parse_one(Str& arg, std::string_view sv) const {
    arg = sv;
    return true;
  }
};

template<>
struct parser<const char*> : parser_one {
  inline bool parse_one(const char*& arg, std::string_view sv) const {
    arg = sv.data();
    return true;
  }
};
} // namespace ivl::cmdline_parsing
