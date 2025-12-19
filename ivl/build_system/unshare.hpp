#pragma once

#include <ivl/linux/raw_syscalls>
#include <ivl/logger>
#include <string_view>

namespace ivl {

  namespace terminate_syscalls {
#define X_PARAMS0()
#define X_PARAMS1(t1, a1) t1 a1
#define X_PARAMS2(t1, a1, ...) t1 a1, X_PARAMS1(__VA_ARGS__)
#define X_PARAMS3(t1, a1, ...) t1 a1, X_PARAMS2(__VA_ARGS__)
#define X_PARAMS4(t1, a1, ...) t1 a1, X_PARAMS3(__VA_ARGS__)
#define X_PARAMS5(t1, a1, ...) t1 a1, X_PARAMS4(__VA_ARGS__)
#define X_PARAMS6(t1, a1, ...) t1 a1, X_PARAMS5(__VA_ARGS__)

#define X_CARGS0()
#define X_CARGS1(t1, a1) a1
#define X_CARGS2(t1, a1, ...) a1, X_CARGS1(__VA_ARGS__)
#define X_CARGS3(t1, a1, ...) a1, X_CARGS2(__VA_ARGS__)
#define X_CARGS4(t1, a1, ...) a1, X_CARGS3(__VA_ARGS__)
#define X_CARGS5(t1, a1, ...) a1, X_CARGS4(__VA_ARGS__)
#define X_CARGS6(t1, a1, ...) a1, X_CARGS5(__VA_ARGS__)

#define X(N, name, ...)                                                                                                \
  inline long name(X_PARAMS##N(__VA_ARGS__)) {                                                                         \
    auto ret = ::ivl::linux::raw_syscalls::name(X_CARGS##N(__VA_ARGS__));                                              \
    if (ret < 0) {                                                                                                     \
      LOG(-ret);                                                                                                       \
      ::ivl::linux::raw_syscalls::exit_group(1);                                                                       \
      asm volatile("ud2" ::: "memory");                                                                                \
    }                                                                                                                  \
    return ret;                                                                                                        \
  }

#include <ivl/linux/syscall_arguments_X>

#undef X_PARAMS0
#undef X_PARAMS1
#undef X_PARAMS2
#undef X_PARAMS3
#undef X_PARAMS4
#undef X_PARAMS5
#undef X_PARAMS6

#undef X_CARGS0
#undef X_CARGS1
#undef X_CARGS2
#undef X_CARGS3
#undef X_CARGS4
#undef X_CARGS5
#undef X_CARGS6
  } // namespace terminate_syscalls

  void full_write(int fd, std::string_view data) {
    while (!data.empty()) data.remove_prefix(ivl::terminate_syscalls::write(fd, data.data(), data.size()));
  }

  void isolate() {
    ivl::terminate_syscalls::unshare(
      CLONE_NEWCGROUP | CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWNS // | CLONE_NEWPID
      | CLONE_NEWTIME | CLONE_NEWUSER | CLONE_NEWUTS
    );

    int fd;

    fd = ivl::terminate_syscalls::openat(AT_FDCWD, "/proc/self/uid_map", O_WRONLY, 0);
    full_write(fd, "0 1000 1");
    ivl::terminate_syscalls::close(fd);

    fd = ivl::terminate_syscalls::openat(AT_FDCWD, "/proc/self/setgroups", O_WRONLY, 0);
    full_write(fd, "deny");
    ivl::terminate_syscalls::close(fd);

    fd = ivl::terminate_syscalls::openat(AT_FDCWD, "/proc/self/gid_map", O_WRONLY, 0);
    full_write(fd, "0 1000 1");
    ivl::terminate_syscalls::close(fd);
  }

} // namespace ivl
