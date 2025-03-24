#pragma once

#include <string>

namespace ivl::color {

  struct RGB { uint8_t r, g, b; };

  std::string_view rst = "\x1B[0 m";
  std::string fg8(uint8_t r, uint8_t g, uint8_t b){
    return "\x1B[38;5;";
  }
  
} // namespace ivl::color::b24
