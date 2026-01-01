#pragma once

#include <ivl/linux/file_descriptor>
#include <ivl/linux/terminate_syscalls>
#include <ivl/linux/throwing_syscalls>
#include <format>
#include <string>

namespace ivl::linux {

// TODO: this should be validated on startup via sysconf(_SC_PAGE_SIZE)
inline constexpr size_t page_size = 4096;

// TODO: change arg into null-terminated-string-view
// TODO: change ret to mmap_region probably
inline std::string read_file(const char* path) {
  owned_file_descriptor fd{throwing_syscalls::open(path, O_RDONLY, 0)};
  struct stat statbuf;
  throwing_syscalls::fstat(fd.get(), &statbuf);
  auto ptr = throwing_syscalls::mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd.get(), 0);
  std::string ret((const char*)ptr, statbuf.st_size);
  throwing_syscalls::munmap(ptr, statbuf.st_size);
  return ret;
}

inline owned_file_descriptor create_tmpfs() {
  alignas(16) char stack[1ULL << 12];

  clone_args clone3_args{};
  clone3_args.flags =
    CLONE_CLEAR_SIGHAND | CLONE_VM | CLONE_FILES | CLONE_VFORK | CLONE_NEWNS | CLONE_NEWUSER // | CLONE_NEWPID
    ;
  clone3_args.exit_signal = SIGCHLD;
  clone3_args.stack = reinterpret_cast<uintptr_t>(&stack[0]);
  clone3_args.stack_size = sizeof(stack);

  namespace sys = terminate_syscalls;

  struct outcome_t {
    uid_t uid;
    gid_t gid;
    long syscall_return = -1;
    long file_line = -1;
  } outcome;
  outcome.uid = sys::getuid();
  outcome.gid = sys::getgid();
  auto clone3_ret = raw_syscalls::fat_clone3(
    &clone3_args, sizeof(clone3_args), &outcome, +[](void* op) noexcept {
      auto& outcome = *static_cast<outcome_t*>(op);

      // LOG(sys::getuid(), sys::geteuid());

      // show("/proc/self/uid_map");

      {
        auto fd = raw_syscalls::open("/proc/self/uid_map", O_WRONLY, 0);
        if (fd < 0) {
          outcome.syscall_return = fd;
          outcome.file_line = __LINE__ - 3;
          raw_syscalls::exit_group(0);
        }
        char data[30];
        auto len = sprintf(data, "0 %lld 1", (long long)outcome.uid);
        auto write_ret = raw_syscalls::write(fd, data, len);
        if (write_ret < 0) {
          outcome.syscall_return = write_ret;
          outcome.file_line = __LINE__ - 3;
          raw_syscalls::exit_group(0);
        }
        auto close_ret = raw_syscalls::close(fd);
        if (close_ret < 0) {
          outcome.syscall_return = close_ret;
          outcome.file_line = __LINE__ - 3;
          raw_syscalls::exit_group(0);
        }
      }

      {
        auto fd = raw_syscalls::open("/proc/self/setgroups", O_WRONLY, 0);
        if (fd < 0) {
          outcome.syscall_return = fd;
          outcome.file_line = __LINE__ - 3;
          raw_syscalls::exit_group(0);
        }
        std::string_view data = "deny";
        auto write_ret = raw_syscalls::write(fd, data.data(), data.size());
        if (write_ret < 0) {
          outcome.syscall_return = write_ret;
          outcome.file_line = __LINE__ - 3;
          raw_syscalls::exit_group(0);
        }
        auto close_ret = raw_syscalls::close(fd);
        if (close_ret < 0) {
          outcome.syscall_return = close_ret;
          outcome.file_line = __LINE__ - 3;
          raw_syscalls::exit_group(0);
        }
      }

      {
        auto fd = raw_syscalls::open("/proc/self/gid_map", O_WRONLY, 0);
        if (fd < 0) {
          outcome.syscall_return = fd;
          outcome.file_line = __LINE__ - 3;
          raw_syscalls::exit_group(0);
        }
        char data[30];
        auto len = sprintf(data, "0 %lld 1", (long long)outcome.gid);
        auto write_ret = raw_syscalls::write(fd, data, len);
        if (write_ret < 0) {
          outcome.syscall_return = write_ret;
          outcome.file_line = __LINE__ - 3;
          raw_syscalls::exit_group(0);
        }
        auto close_ret = raw_syscalls::close(fd);
        if (close_ret < 0) {
          outcome.syscall_return = close_ret;
          outcome.file_line = __LINE__ - 3;
          raw_syscalls::exit_group(0);
        }
      }

      raw_syscalls::ivl_start_logging();
      auto mount_ret = raw_syscalls::mount("tmpfs", "/proc", "tmpfs", 0, (void*)"mode=777,uid=0,gid=0");
      raw_syscalls::ivl_stop_logging();
      if (mount_ret < 0) {
        outcome.syscall_return = mount_ret;
        outcome.file_line = __LINE__ - 3;
        raw_syscalls::exit_group(0);
      }

      raw_syscalls::ivl_start_logging();
      auto open_ret = raw_syscalls::open("/proc", O_RDONLY | O_DIRECTORY, 0);
      raw_syscalls::ivl_stop_logging();
      if (open_ret < 0) {
        outcome.syscall_return = open_ret;
        outcome.file_line = __LINE__ - 3;
        raw_syscalls::exit_group(0);
      }

      outcome.syscall_return = open_ret;
      raw_syscalls::exit_group(0);
    }
  );

  if (clone3_ret < 0) {
    throw std::runtime_error(std::format("clone3 failed with error: {}", clone3_ret));
  }

  int wstatus = -1;
  auto wait4_ret = raw_syscalls::wait4(clone3_ret, &wstatus, 0, nullptr);
  if (wait4_ret < 0) {
    throw std::runtime_error(std::format("wait4 failed with error: {}", wait4_ret));
  }

  if (wstatus != 0) {
    throw std::runtime_error(std::format("unexpected exit code: {}", wstatus));
  }

  if (outcome.syscall_return < 0) {
    throw std::runtime_error(
      std::format(
        "unexpected syscall error {} in occurred child process in line {}", outcome.syscall_return, outcome.file_line
      )
    );
  }

  return owned_file_descriptor((int)outcome.syscall_return);
}

} // namespace ivl::linux
