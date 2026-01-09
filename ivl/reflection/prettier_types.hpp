#pragma once

#include <meta>
#include <string_view>
#include <string>
#include <filesystem>

namespace ivl::reflection {
  consteval std::string_view display_string_of(std::meta::info r){
#define X(...) if (is_type(r) && dealias(r) == dealias(^^__VA_ARGS__)) return #__VA_ARGS__
    X(std::string);
    X(std::filesystem::path);
#undef X
    return std::meta::display_string_of(r);
  }
} // ivl::reflection
