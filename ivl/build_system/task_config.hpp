#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ivl::build_system {
struct task_config {
  std::string identifier;

  std::vector<std::filesystem::path> inputs;
  std::vector<std::filesystem::path> outputs;

  // i think wd shouldn't matter most of the time
  std::filesystem::path process_working_directory;
  std::filesystem::path process_pathname;
  std::vector<std::string> process_argv;
  std::vector<std::string> process_envp;
};
} // namespace ivl::build_system
