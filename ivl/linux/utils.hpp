#pragma once

#include <ivl/linux/terminate_syscalls>
#include <string>

namespace ivl::linux {

// TODO: change arg into null-terminated-string-view
// TODO: change ret to mmap_region probably
// TODO: better error reporting
std::string read_file(const char* path) {
  namespace sys = terminate_syscalls;
  auto fd = sys::open(path, O_RDONLY, 0);
  struct stat statbuf;
  sys::fstat(fd, &statbuf);
  auto ptr = sys::mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  std::string ret((const char*)ptr, statbuf.st_size);
  sys::munmap(ptr, statbuf.st_size);
  return ret;
}

} // namespace ivl::linux
