#pragma once

#include "v2"

namespace ivl::algorithm {
template<typename It, typename Cmp = std::compare_three_way>
void monotonic_join(It a, It b, It c, Cmp&& order = {}) {
  return monotonic_join_v2(a, b, c, order);
}
} // namespace ivl::algorithm
