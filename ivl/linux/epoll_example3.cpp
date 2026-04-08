#include <ivl/linux/epoll>
#include <ivl/linux/terminate_syscalls>
#include <ivl/linux/throwing_syscalls>
#include <ivl/logger>
#include <ivl/reflection/json>
#include <sys/epoll.h>
#include <sys/wait.h>

// TODO: gcc bug, remove when fixed: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124648
#define pre(...)

int main() {
  namespace sys = ivl::linux::throwing_syscalls;

  ivl::linux::epoll_file_descriptor efd(sys::semantic);
  size_t running_count = 0;
  static constexpr size_t parallelism_limit = 3;

  auto add_fd = [&](ivl::linux::file_descriptor fd) pre(running_count < parallelism_limit) {
    efd.ctl_add_fd(sys::semantic, fd, ivl::linux::epoll_events_enum::EPOLLIN);
    ++running_count;
  };

  auto remove_fd = [&](ivl::linux::file_descriptor fd) pre(running_count > 0) {
    efd.ctl_del(sys::semantic, fd);
    --running_count;
  };

  auto launch_process = [&, &running_count](int arg) pre(running_count < parallelism_limit) {
    alignas(16) char stack[1ULL << 12];

    int pidfd = -1;
    clone_args clone3_args{
      .flags = CLONE_CLEAR_SIGHAND | CLONE_VM | CLONE_VFORK | CLONE_PIDFD,
      .pidfd = reinterpret_cast<uint64_t>(&pidfd),
      .exit_signal = 0,
      .stack = reinterpret_cast<uintptr_t>(&stack[0]),
      .stack_size = sizeof(stack),
    };

    std::string ex = std::format("echo 'running process {0}'; sleep 1; exit {0}", arg);
    std::vector<const char*> argv;
    argv.push_back("/usr/bin/bash");
    argv.push_back("-c");
    argv.push_back(ex.data());
    argv.push_back(nullptr);

    auto pid = sys::fat_clone3(
      &clone3_args, sizeof(clone3_args), argv.data(), +[](void* argv) noexcept {
        namespace sys = ivl::linux::terminate_syscalls;
        const char* envp[] = {"PATH=/usr/bin", "LC_ALL=C", nullptr};
        sys::execve("/usr/bin/bash", (const char* const*)argv, envp);
      }
    );

    add_fd(ivl::linux::file_descriptor(pidfd));
  };

  auto wait_for_exit = [&, &running_count] pre(running_count > 0) {
    epoll_event ev{};
    auto ret = efd.wait_block_forever(sys::semantic, {&ev, 1});
    // LOG(running_count, ret);
    contract_assert(ret == 1);
    return ev.data.fd;
  };

  auto reap_process = [&, &running_count](int pidfd) pre(running_count > 0) {
    // LOG(pidfd);
    siginfo_t info{};
    auto wait_ret = sys::waitid(P_PIDFD, pidfd, (siginfo*)&info, WEXITED | WNOHANG, nullptr);
    if (info.si_pid == 0) return;
    contract_assert(info.si_code == CLD_EXITED);
    remove_fd(ivl::linux::file_descriptor(pidfd));
    // return info.si_status;
  };

  std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8};
  for (auto el : vec) {
    while (running_count == parallelism_limit) reap_process(wait_for_exit());
    launch_process(el);
  }

  while (running_count) reap_process(wait_for_exit());
}
