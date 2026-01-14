#pragma once

#include <meta>
#include <span>
#include <vector>

namespace ivl {
template <typename E>
  requires std::is_enum_v<E>
consteval std::span<const E> enumerators() {
  std::vector<E> ret;
  for (auto e : enumerators_of(^^E)) ret.push_back(extract<E>(e));
  return define_static_array(ret);
}
} // namespace ivl
