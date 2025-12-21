#pragma once

#include <ivl/linux/terminate_syscalls>
#include <string_view>

namespace ivl {

  void full_write(int fd, std::string_view data) {
    while (!data.empty()) data.remove_prefix(ivl::linux::terminate_syscalls::write(fd, data.data(), data.size()));
  }

  void isolate() {
    ivl::linux::terminate_syscalls::unshare(
      CLONE_NEWCGROUP | CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWNS // | CLONE_NEWPID
      | CLONE_NEWTIME | CLONE_NEWUSER | CLONE_NEWUTS
    );

    int fd;

    fd = ivl::linux::terminate_syscalls::openat(AT_FDCWD, "/proc/self/uid_map", O_WRONLY, 0);
    full_write(fd, "0 1000 1");
    ivl::linux::terminate_syscalls::close(fd);

    fd = ivl::linux::terminate_syscalls::openat(AT_FDCWD, "/proc/self/setgroups", O_WRONLY, 0);
    full_write(fd, "deny");
    ivl::linux::terminate_syscalls::close(fd);

    fd = ivl::linux::terminate_syscalls::openat(AT_FDCWD, "/proc/self/gid_map", O_WRONLY, 0);
    full_write(fd, "0 1000 1");
    ivl::linux::terminate_syscalls::close(fd);
  }

} // namespace ivl
