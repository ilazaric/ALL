#pragma once

#include <ivl/build_system/task_config>
#include <ivl/crypto/blake3>
#include <filesystem>
#include <vector>

namespace ivl::build_system {
// if fingerprint changes, we need to rebuild
struct task_fingerprint {
  struct file_info {
    std::filesystem::path path;
    std::filesystem::file_time_type mtime;
    crypto::blake3::input_chain_value hash;
  };
  task_config config;
  std::vector<file_info> infos;
};
} // namespace ivl::build_system
