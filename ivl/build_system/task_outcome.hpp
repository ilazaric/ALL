#pragma once

#include <sys/resource.h>
#include <map>
#include <signal.h>
#include <string>

namespace ivl::build_system {
struct task_outcome {
  std::string identifier;

  // whatever waitid() fills in the infop ptr, exit status etc
  siginfo_t info;

  std::string stdout;
  std::string stderr;

  std::chrono::nanoseconds duration;

  struct rusage end_rusage;
  std::map<std::string, std::string> end_cgroup_files;
};
} // namespace ivl::build_system
