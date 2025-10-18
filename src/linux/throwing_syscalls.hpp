#pragma once

#include <ivl/linux/file_descriptor>
#include <ivl/linux/raw_syscalls>
#include <string>

namespace ivl::linux::throwing {

  struct open_error {
    std::string filename;
    int         flags;
    mode_t      mode;
  };

  owned_file_descriptor open(const char* filename, int flags, mode_t mode) {
    auto ret = raw_syscalls::open(filename, flags, mode);
    if (ret < 0) throw open_error{filename, flags, mode};
    return owned_file_descriptor{(int)ret};
  }

} // namespace ivl::linux::throwing
