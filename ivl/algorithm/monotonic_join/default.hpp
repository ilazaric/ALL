#pragma once

#include "v2"

namespace ivl::algorithm {
template<typename It>
void monotonic_join(It a, It b, It c) {
  return monotonic_join_v2(a, b, c);
}
} // namespace ivl::algorithm
