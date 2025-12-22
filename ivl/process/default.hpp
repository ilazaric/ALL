#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <ivl/linux/kernel_result>
#include <ivl/linux/raw_syscalls>

namespace ivl {

struct process {
  process();

  // // TODO: might be wrong api
  // explicit process(const process_config&);

  process(const process&) = delete;
  process(process&& o) : pid(o.pid) { o.pid = 0; }

  process& operator=(const process&) = delete;
  process& operator=(process&& o) {
    std::swap(pid, o.pid);
    return *this;
  }

  linux::or_syscall_error<long> wait() {
    if (pid == 0) return linux::or_syscall_error<long>{-EINVAL};
    int wstatus;
    auto ret = ivl::linux::raw_syscalls::wait4(pid, &wstatus, 0, nullptr);
    if (ret < 0) return linux::or_syscall_error<long>{ret};
    // TODO
    return linux::or_syscall_error<long>{wstatus};
  }

  ~process() {
    // if (kill_on_destruction) kill(SIGKILL);
    // if (!wait_on_destruction) return;
    (void)wait();
  }

  explicit process(pid_t pid) : pid(pid) {}

private:
  pid_t pid;
  // friend class process_config;
};

struct process_config {
  std::filesystem::path pathname;
  std::vector<std::string> argv;
  std::map<std::string, std::string> envp;
  std::function<void()> pre_exec_setup = [] {};
  uint64_t cgroup = 0;
  bool isolate_all = false;
  bool share_fds = false;
  // TODO
  // bool                               die_on_parent_death = true;
  // bool                               kill_on_destruction = false;
  // bool                               wait_on_destruction = true;

  void pre_exec(auto&& callable) {
    pre_exec_setup = [first = std::move(pre_exec_setup), second = std::move(callable)] {
      first();
      second();
    };
  }

  // TODO: builder pattern
  // TODO: copy env
  // TODO: seccomp
  // TODO: cgroups
  // TODO: std{in,out,err} of child
  // actually, might want more generic approach for fds

  // If error, it is due to clone3 syscall.
  // Errors related to execve are not seen.
  // Nope, now includes execve errors.
  // TODO
  linux::or_syscall_error<process> clone_and_exec() const {
    LOG(pathname);
    const char* actual_pathname = pathname.c_str();
    auto actual_exefd = linux::raw_syscalls::open(actual_pathname, O_RDONLY, 0);
    if (actual_exefd < 0) return linux::or_syscall_error<process>(actual_exefd);
    std::vector<const char*> actual_argv(argv.size() + 1, nullptr);
    std::vector<const char*> actual_envp;  //{envp.size() + 1, nullptr};
    std::vector<std::string> envp_storage; //{envp.size()};
    for (auto&& [k, v] : envp) {
      envp_storage.push_back(k + "=" + v);
      actual_envp.push_back(envp_storage.back().c_str());
    }
    actual_envp.push_back(nullptr);
    long actual_err = 0;
    for (size_t i = 0; i < argv.size(); ++i) actual_argv[i] = argv[i].c_str();
    using T = std::tuple<long, const char* const*, const char* const*, long*, const std::function<void()>*>;
    T exec_args{actual_exefd, &actual_argv[0], &actual_envp[0], &actual_err, &pre_exec_setup};

    alignas(16) char stack[1ULL << 12];

    LOG(isolate_all);
    clone_args clone3_args{
      // TODO: https://ewontfix.com/7/
      .flags = CLONE_CLEAR_SIGHAND | CLONE_VM | CLONE_VFORK | (cgroup ? CLONE_INTO_CGROUP : 0) |
               (isolate_all ? CLONE_NEWCGROUP | CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWNS | CLONE_NEWPID |
                                CLONE_NEWTIME | CLONE_NEWUSER | CLONE_NEWUTS
                            : 0) |
               (share_fds ? CLONE_FILES : 0),
      .pidfd{},
      .child_tid{},
      .parent_tid{},
      .exit_signal = SIGCHLD,
      .stack = reinterpret_cast<uintptr_t>(&stack[0]),
      .stack_size = sizeof(stack),
      .tls{},
      .set_tid{},
      .set_tid_size{},
      .cgroup{cgroup},
    };
    auto ret = linux::raw_syscalls::fat_clone3(
      &clone3_args, sizeof(clone3_args), &exec_args, +[](void* child_arg) noexcept {
        // TODO
        auto [exefd, argv, envp, err, pre_exec_setup] = *reinterpret_cast<const T*>(child_arg);
        *err = ivl::linux::raw_syscalls::prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
        if (*err < 0) goto bad;
        *err = -1; // TODO
        (*pre_exec_setup)();
        *err = 0;
        *err = ivl::linux::raw_syscalls::execveat(exefd, "", argv, envp, AT_EMPTY_PATH);
        LOG(*err);
      bad:
        ivl::linux::raw_syscalls::exit_group(1);
        // ivl::linux::raw_syscalls::ud2();
      }
    );
    if (ret < 0) return linux::or_syscall_error<process>(ret);
    if (actual_err < 0) {
      process p(ret); // just to wait
      return linux::or_syscall_error<process>(actual_err);
    }
    return linux::or_syscall_error<process>(ret);
  }
};

} // namespace ivl
