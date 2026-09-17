#pragma once

#warning "TODO"

#include <ivl/build_system/task_executor>
#include <queue>

namespace ivl::build_system {
struct throttled_task_config {
  task_config config;
  std::map<std::string, std::size_t> resources;
};

struct throttled_task_executor {
  task_executor underlying;
  std::map<std::string, std::size_t> resources;
  std::queue<throttled_task_config> remaining;

  void launch_task(const task_config& task, const task_limits& limits = {}) {
    auto cpu = limits.cpu_max_percentage.value_or(underlying.default_cpu_max_percentage);
    contract_assert(cpu <= max_cpu_percentage);
    if (current_cpu_percentage + cpu <= max_cpu_percentage) {
      underlying.launch_task(task, limits);
      current_cpu_percentage += cpu;
    } else {
      remaining.push({task, limits});
    }
  }

  void drain_queue() {
    while (!remaining.empty()) {
      auto&& [task, limits] = remaining.front();
      auto cpu = limits.cpu_max_percentage.value_or(underlying.default_cpu_max_percentage);
      if (current_cpu_percentage + cpu > max_cpu_percentage) break;
      underlying.launch_task(task, limits);
      remaining.pop();
    }
  }

  task_outcome wait_for_death() { auto ret = underlying.wait_for_death(); }
};
} // namespace ivl::build_system
