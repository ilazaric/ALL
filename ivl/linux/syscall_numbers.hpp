#pragma once

// TODO: replace with ivl null terminated string view
#include <string_view>

namespace ivl::linux {

enum class syscall_number {
#define X(nr, name) name = nr,
#include <ivl/linux/syscall_numbers_X>
};

// If the argument is invalid returns default-constructed std::string_view.
constexpr std::string_view syscall_name(syscall_number nr) noexcept {
  switch (nr) {
  default:
    return {};
#define X(nr, name)                                                                                                    \
  case syscall_number::name:                                                                                           \
    return #name;
#include <ivl/linux/syscall_numbers_X>
  }
}

constexpr bool is_valid_syscall_number(int nr) noexcept {
  switch (nr) {
  default:
    return false;
#define X(nr, name)                                                                                                    \
  case nr:                                                                                                             \
    return true;
#include <ivl/linux/syscall_numbers_X>
  }
}

} // namespace ivl::linux
