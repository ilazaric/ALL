#pragma once

#include <print>

template <int A, int B, int C>
int magic_fn(int arg) {
  int res = 0;
  if constexpr (A > 0) res += magic_fn<A-1, B, C>(arg);
  if constexpr (B > 0) res += magic_fn<A, B-1, C>(arg);
  if constexpr (C > 0) res += magic_fn<A, B, C-1>(arg);
  return res;
}

void func() { std::println("{}", magic_fn<30, 30, 30>(42)); }
