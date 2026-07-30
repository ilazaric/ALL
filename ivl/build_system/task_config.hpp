#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ivl::build_system {
struct task_config {
  std::string identifier;

  std::vector<std::filesystem::path> inputs;
  std::vector<std::filesystem::path> outputs;
  // Running this task also means refreshing unordered_dependencies,
  // but they dont have to happen before this is started.
  // TODO
  // std::vector<std::filesystem::path> unordered_dependencies;

  // i think wd shouldn't matter most of the time
  std::filesystem::path process_working_directory;
  std::filesystem::path process_pathname;
  std::vector<std::string> process_argv;
  std::vector<std::string> process_envp;

  // If true, this task is always rerun if an output is important.
  // This can be used to model running binaries.
  // TODO
  // bool always_dirty = false;

  bool operator==(const task_config&) const = default;
};
} // namespace ivl::build_system
