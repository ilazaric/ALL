#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <ivl/exception>
#include <ivl/linux/file_descriptor>
#include <ivl/linux/kernel_result>
#include <ivl/linux/raw_syscalls>
#include <ivl/linux/terminate_syscalls>
#include <ivl/linux/throwing_syscalls>
#include <ivl/linux/utils>
#include <ivl/logger>

namespace ivl {

struct process {
  pid_t pid;

  process() : pid(0) {}

  process(const process&) = delete;
  process(process&& o) : pid(o.pid) { o.pid = 0; }

  process& operator=(const process&) = delete;
  process& operator=(process&& o) {
    std::swap(pid, o.pid);
    return *this;
  }

  linux::or_syscall_error<long> wait() {
    if (pid == 0) return linux::or_syscall_error<long>{-EINVAL};
    // TODO: maybe wrap this into a struct that understands WIFEXITED et al
    int wstatus;
    auto ret = ivl::linux::raw_syscalls::wait4(pid, &wstatus, 0, nullptr);
    if (ret < 0) return linux::or_syscall_error<long>{ret};
    return linux::or_syscall_error<long>{wstatus};
  }

  ~process() { (void)wait(); }

  explicit process(pid_t pid) : pid(pid) {}
};

struct process_config {
  // TODO: probably just std::string
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
    pre_exec_setup = [first = std::move(pre_exec_setup), second = FWD(callable)] {
      // TODO: consider changing pre_exec_setup to std::vector<std::function<void()>>
      first();
      second();
    };
  }

  // TODO: builder pattern
  // TODO: copy env
  // TODO: seccomp
  // TODO: cgroups
  // TODO: std{in,out,err} of child
  // ....: actually, might want more generic approach for fds

  // If error, it is due to clone3 syscall.
  // Errors related to execve are not seen.
  // Nope, now includes execve errors.
  // TODO
  linux::or_syscall_error<process> clone_and_exec() const {
    const char* actual_pathname = pathname.c_str();
    // TODO: this might actually be bad for isolation
    // ....: but i havent found a way to exploit it
    // ....: still might be prudent to revert to execve()
    // UPDT: reverted to execve for now
    // auto actual_exefd = linux::raw_syscalls::open(actual_pathname, O_RDONLY | O_CLOEXEC, 0);
    // if (actual_exefd < 0) return linux::or_syscall_error<process>(actual_exefd);
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

    struct exec_args_t {
      const char* pathname;
      const char* const* argv;
      const char* const* envp;
      long* err;
      const std::function<void()>* pre_exec_setup;
    };
    exec_args_t exec_args{
      .pathname = actual_pathname,
      .argv = &actual_argv[0],
      .envp = &actual_envp[0],
      .err = &actual_err,
      .pre_exec_setup = &pre_exec_setup
    };

    alignas(16) char stack[1ULL << 12];

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
        auto& exec_args = *static_cast<exec_args_t*>(child_arg);
        // TODO: this might not be correct if parent dies before it?
        // TODO: erm what if the process is moved out of thread and the thread dies? man prctl
        // UPDT: killing prctl for now bc it kinda sucks
        // *exec_args.err = ivl::linux::raw_syscalls::prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
        // if (*exec_args.err < 0) goto bad;
        *exec_args.err = -1; // TODO
        (*exec_args.pre_exec_setup)();
        *exec_args.err = 0;
        *exec_args.err = ivl::linux::raw_syscalls::execve(exec_args.pathname, exec_args.argv, exec_args.envp);
      bad:
        ivl::linux::raw_syscalls::exit_group(1);
        // ivl::linux::raw_syscalls::ud2();
      }
    );
    // ivl::linux::raw_syscalls::close(actual_exefd);
    if (ret < 0) return linux::or_syscall_error<process>(ret);
    if (actual_err < 0) {
      process p(ret); // just to wait
      return linux::or_syscall_error<process>(actual_err);
    }
    return linux::or_syscall_error<process>(ret);
  }
};

// stdin = /dev/null
// stdout = memfd_create
// stderr left alone
struct process_function {
  std::filesystem::path pathname;
  std::vector<std::string> argv;
  std::vector<std::string> envp;

  std::string operator()(auto&&... args) const {
    namespace sys = linux::throwing_syscalls;
    auto str_args = std::array{std::format("{}", args)...};
    std::vector<const char*> pass_args(argv.size() + str_args.size() + 2, nullptr);
    pass_args[0] = pathname.c_str();
    for (size_t i = 0; i < argv.size(); ++i) pass_args[i + 1] = argv[i].c_str();
    for (size_t i = 0; i < str_args.size(); ++i) pass_args[i + argv.size() + 1] = str_args[i].c_str();
    EXCEPTION_CONTEXT(
      "pathname={} argv={} envp={}", pathname, std::span(pass_args).subspan(1).first(pass_args.size() - 2), envp
    );
    std::vector<const char*> pass_envp(envp.size() + 1, nullptr);
    for (size_t i = 0; i < envp.size(); ++i) pass_envp[i] = envp[i].c_str();

    linux::owned_file_descriptor stdoutfd(sys::memfd_create("process-output", 0));

    long actual_err = 0;
    struct exec_args_t {
      const char* pathname;
      const char* const* argv;
      const char* const* envp;
      pid_t ppid;
      int stdoutfd;
    };
    exec_args_t exec_args{
      .pathname = pathname.c_str(),
      .argv = pass_args.data(),
      .envp = pass_envp.data(),
      .ppid = (pid_t)sys::getpid(),
      .stdoutfd = stdoutfd.get(),
    };
    alignas(16) char stack[1ULL << 12];

    clone_args clone3_args{
      // TODO: https://ewontfix.com/7/
      .flags = CLONE_CLEAR_SIGHAND | CLONE_VM | CLONE_VFORK,
      .pidfd{},
      .child_tid{},
      .parent_tid{},
      .exit_signal = SIGCHLD,
      .stack = reinterpret_cast<uintptr_t>(&stack[0]),
      .stack_size = sizeof(stack),
      .tls{},
      .set_tid{},
      .set_tid_size{},
      .cgroup{},
    };
    auto pid = sys::fat_clone3(
      &clone3_args, sizeof(clone3_args), &exec_args, +[](void* child_arg) noexcept {
        namespace sys = linux::terminate_syscalls;
        auto& exec_args = *static_cast<exec_args_t*>(child_arg);
        sys::prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
        if (sys::getppid() != exec_args.ppid) sys::exit_group(1);
        if (exec_args.stdoutfd != 1) {
          sys::dup2(exec_args.stdoutfd, 1);
          sys::close(exec_args.stdoutfd);
        }
        auto dev_null = sys::open("/dev/null", O_RDONLY, 0);
        if (dev_null != 0) {
          sys::dup2(dev_null, 0);
          sys::close(dev_null);
        }
        sys::execve(exec_args.pathname, exec_args.argv, exec_args.envp);
      }
    );

    int wstatus;
    auto ret = sys::wait4(pid, &wstatus, 0, nullptr);
    if (wstatus != 0) throw ivl::base_exception(std::format("process exited with status {}", ret));
    return linux::read_file(stdoutfd);
  }
};

} // namespace ivl
