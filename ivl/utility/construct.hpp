#pragma once

#include <ivl/utility/forward>

// TODO: name might be too generic in such a wide namespace

namespace ivl {
template<typename T>
struct construct_t {
  static constexpr T operator()(auto&&... args) { return T(FWD(args)...); }
};
template<typename T>
inline constexpr construct_t<T> construct;
} // namespace ivl
