#pragma once

#include <format>
#include <string>

// https://en.wikipedia.org/wiki/ANSI_escape_code#24-bit

namespace ivl {

std::string foreground(uint8_t r, uint8_t g, uint8_t b) { return std::format("\x1B[38;2;{};{};{}m", r, g, b); }

std::string background(uint8_t r, uint8_t g, uint8_t b) { return std::format("\x1B[48;2;{};{};{}m", r, g, b); }

std::string_view reset() { return "\x1B[m"; }

} // namespace ivl
