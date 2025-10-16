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
    std::filesystem::path              pathname;
    std::vector<std::string>           argv;
    std::map<std::string, std::string> envp;
    std::function<void()>              pre_exec_setup = [] {};
    // TODO
    // bool                               die_on_parent_death = true;
    // bool                               kill_on_destruction = false;
    // bool                               wait_on_destruction = true;

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
      const char*              actual_pathname = pathname.c_str();
      std::vector<const char*> actual_argv(argv.size() + 1, nullptr);
      std::vector<const char*> actual_envp;  //{envp.size() + 1, nullptr};
      std::vector<std::string> envp_storage; //{envp.size()};
      for (auto&& [k, v] : envp) {
        envp_storage.push_back(k + "=" + v);
        actual_envp.push_back(envp_storage.back().c_str());
      }
      actual_envp.push_back(nullptr);
      long actual_err = 0;
      for (size_t i = 0; i < argv.size(); ++i)
        actual_argv[i] = argv[i].c_str();
      using T = std::tuple<const char*, const char* const*, const char* const*, long*, const std::function<void()>*>;
      T exec_args{actual_pathname, &actual_argv[0], &actual_envp[0], &actual_err, &pre_exec_setup};

      alignas(16) char stack[1ULL << 12];

      clone_args clone3_args{
        .flags = CLONE_VM | CLONE_VFORK,
        .pidfd{},
        .child_tid{},
        .parent_tid{},
        .exit_signal = SIGCHLD,
        .stack       = reinterpret_cast<uintptr_t>(&stack[0]),
        .stack_size  = (1ULL << 12),
        .tls{},
        .set_tid{},
        .set_tid_size{},
        .cgroup{},
      };
      auto ret = linux::raw_syscalls::fat_clone3(
        &clone3_args, sizeof(clone3_args), &exec_args, +[](void* child_arg) noexcept {
          // TODO: prctl suicide
          auto [pathname, argv, envp, err, pre_exec_setup] = *reinterpret_cast<const T*>(child_arg);
          (*pre_exec_setup)();
          *err = ivl::linux::raw_syscalls::execve(pathname, argv, envp);
          ivl::linux::raw_syscalls::exit_group(1);
          ivl::linux::raw_syscalls::ud2();
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
