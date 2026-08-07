#pragma once

#include <ivl/linux/file_descriptor>
#include <ivl/linux/throwing_syscalls>

// man proc_pid_pagemap

namespace ivl::linux {
struct pagemap_value {
  uint64_t value;

  uint64_t bit(uint64_t pos) const { return (value >> pos) & 1; }

  bool is_in_ram() const { return bit(63); }
  bool is_in_swap() const { return bit(62); }
  bool is_file_or_shared() const { return bit(61); }
  // 60-58 reserved
  bool is_write_protected() const { return bit(57); }
  bool is_exclusive() const { return bit(56); }
  bool is_soft_dirty() const { return bit(55); }

  uint64_t page_frame_number() const { return value & ((1ULL << 55) - 1); }
};

pagemap_value pagemap(const void* addr, file_descriptor fd) {
  namespace sys = ivl::linux::throwing_syscalls;
  uint64_t loc = reinterpret_cast<uint64_t>(addr) / 4096 * 8;
  sys::lseek(fd.get(), loc, SEEK_SET);
  uint64_t ret;
  auto cnt = sys::read(fd.get(), (char*)&ret, 8);
  contract_assert(cnt == 8);
  return {ret};
}

pagemap_value pagemap(const void* addr) {
  namespace sys = ivl::linux::throwing_syscalls;
  ivl::linux::owned_file_descriptor fd{sys::open("/proc/self/pagemap", O_RDONLY, 0)};
  return pagemap(addr, fd);
}
} // namespace ivl::linux
