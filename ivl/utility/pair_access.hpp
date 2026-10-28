#pragma once

namespace ivl {
struct first_t {
  template<typename T>
  static constexpr decltype(auto) operator()(T&& arg) {
    return static_cast<T&&>(arg).first;
  }
};
inline constexpr first_t first;

struct second_t {
  template<typename T>
  static constexpr decltype(auto) operator()(T&& arg) {
    return static_cast<T&&>(arg).second;
  }
};
inline constexpr second_t second;
}; // namespace ivl
