#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

namespace ivl::build_system {
struct task_limits {
  // if unset uses executor defaults
  std::optional<std::size_t> cpu_max_percentage;
  std::optional<std::size_t> memory_limit;
  std::optional<std::chrono::nanoseconds> time_limit;
};
} // namespace ivl::build_system
