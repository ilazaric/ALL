#pragma once

#include <sys/resource.h>
#include <map>
#include <string>

namespace ivl::build_system {
struct task_outcome {
  std::string identifier;

  // TODO: also code
  int exit_status;
  std::string stdout;
  std::string stderr;

  std::chrono::nanoseconds duration;

  struct rusage end_rusage;
  std::map<std::string, std::string> end_cgroup_files;
};
} // namespace ivl::build_system
