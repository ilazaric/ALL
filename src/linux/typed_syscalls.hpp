#pragma once

#include <ivl/linux/file_descriptor>
#include <ivl/linux/kernel_result>
#include <ivl/linux/raw_syscalls>

namespace ivl::linux {

  enum class open_flags : int {
    rdonly,
    wronly,
    rdwr,
    append,
    async,
    cloexec,
    creat,
    direct,
    directory,
    dsync,
    excl,
    largefile,
    noatime,
    noctty,
    nofollow,
    nonblock,
    ndelay,
    path,
    sync,
    tmpfile,
    trunc,
  };

  enum class open_mode : mode_t {
    // TODO
  };

  auto open(const char* filename, int flags, mode_t mode) {
    return or_syscall_error<owned_file_descriptor>(raw_syscalls::open(filename, flags, mode));
  }

  // TODO: buf, count should be wrapped into a string_view or span<const char> or span<const byte>
  auto write(file_descriptor fd, const char* buf, size_t count) {
    return or_syscall_error<long>(raw_syscalls::write(fd, buf, count));
  }

  // auto mmap(void* addr, size_t len, int prot, int flags, file_descriptor fd, ){
  //   return or_syscall_error<owned_mmap_region>(raw_syscalls::mmap());
  // }

  // kernel_result read();
  // kernel_result close();

} // namespace ivl::linux
