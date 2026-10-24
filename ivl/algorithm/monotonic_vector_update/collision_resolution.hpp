#pragma once

#include <ivl/utility/enum>

namespace ivl::algorithm {
enum class monotonic_vector_update_collision_resolution_raw {
  preserve = 1,
  erase = 2,
};

using monotonic_vector_update_collision_resolution = checked_enum<monotonic_vector_update_collision_resolution_raw>;
} // namespace ivl::algorithm
