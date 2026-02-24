#pragma once

#include <string_view>

namespace ivl::util {

  // a string that can be passed via template args
  template <unsigned N>
  struct fixed_string {
    char buf[N + 1]{};
    consteval fixed_string(char const* s) {
      for (unsigned i = 0; i != N; ++i)
        buf[i] = s[i];
    }
    consteval                  operator char const*() const { return buf; }
    constexpr std::string_view view() const { return std::string_view(&buf[0], N); }
    constexpr size_t           size() const { return N; }
  };
  template <unsigned N>
  fixed_string(char const (&)[N]) -> fixed_string<N - 1>;

} // namespace ivl::util
