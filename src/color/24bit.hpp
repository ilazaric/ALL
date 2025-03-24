#pragma once

#include <string>

namespace ivl::color {

  std::string_view rst = "\x1B[49m\x1B[39m\x1B[K";
  std::string fg24(uint8_t r, uint8_t g, uint8_t b){
    return "\x1B[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
  }
  std::string bg24(uint8_t r, uint8_t g, uint8_t b){
    return "\x1B[48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
  }
  
} // namespace ivl::color::b24
