#pragma once

#include <ivl/linux/file_descriptor>
#include <ivl/linux/throwing_syscalls>

namespace ivl::linux {
uint64_t pagemap(const void* addr, file_descriptor fd) {
  namespace sys = ivl::linux::throwing_syscalls;
  uint64_t loc = reinterpret_cast<uint64_t>(addr) / 4096 * 8;
  sys::lseek(fd.get(), loc, SEEK_SET);
  uint64_t ret;
  auto cnt = sys::read(fd.get(), (char*)&ret, 8);
  contract_assert(cnt == 8);
  return ret;
}

uint64_t pagemap(const void* addr) {
  namespace sys = ivl::linux::throwing_syscalls;
  ivl::linux::owned_file_descriptor fd{sys::open("/proc/self/pagemap", O_RDONLY, 0)};
  return pagemap(addr, fd);
}
} // namespace ivl::linux
