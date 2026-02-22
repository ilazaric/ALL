#pragma once

namespace ivl::util {
template <typename Fn>
struct [[nodiscard]] scope_exit {
  Fn fn;
  ~scope_exit() { fn(); }
};
} // namespace ivl
