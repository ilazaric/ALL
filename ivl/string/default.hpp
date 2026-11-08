#pragma once

#include <ivl/utility/forward>
#include <string>

namespace ivl {
struct to_string_t {
  static std::string operator()(auto&& x) {
    if constexpr (requires { FWD(x).to_string(); }) {
      return FWD(x).to_string();
    } else if constexpr (requires { to_string(FWD(x)); }) {
      return to_string(FWD(x));
    } else if constexpr (requires { std::to_string(FWD(x)); }) {
      return std::to_string(FWD(x));
    } else {
      static_assert(false);
    }
  }
};
inline constexpr to_string_t to_string;
} // namespace ivl
