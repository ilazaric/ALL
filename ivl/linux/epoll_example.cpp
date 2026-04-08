// #include <ivl/linux/epoll>
#include <ivl/linux/throwing_syscalls>
#include <ivl/logger>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <ivl/reflection/json>
#include <ivl/linux/syscall_error>

static_assert(alignof(epoll_event) == 1);

int main() {
  namespace sys = ivl::linux::throwing_syscalls;

  auto efd = sys::epoll_create1(0);

  alignas(16) char stack[1ULL << 12];

  int pidfd = -1;
  clone_args clone3_args{
    .flags = CLONE_CLEAR_SIGHAND | CLONE_VM | CLONE_VFORK | CLONE_PIDFD,
    .pidfd = reinterpret_cast<uint64_t>(&pidfd),
    .exit_signal = 0,
    .stack = reinterpret_cast<uintptr_t>(&stack[0]),
    .stack_size = sizeof(stack),
  };

  auto pid = sys::fat_clone3(
    &clone3_args, sizeof(clone3_args), nullptr, +[](void*) noexcept { ivl::linux::raw_syscalls::exit_group(1); }
  );
  LOG(pid);
  LOG(pidfd);

  epoll_event ev{
    .events = EPOLLIN | EPOLLHUP,
  };
  sys::epoll_ctl(efd, EPOLL_CTL_ADD, pidfd, &ev);

  epoll_event ev2{};
  auto ret = sys::epoll_wait(efd, &ev2, 1, 0);
  LOG(ret);
  LOG(auto(ev2.events));
  LOG(EPOLLIN);

  // while (true);

  siginfo_t info;
  // rusage ru;
  auto wait_ret = sys::waitid(P_PIDFD, pidfd, (siginfo*)&info,
                              0
                              | WEXITED
                              // | WNOHANG
                              | __WCLONE
                              , nullptr);
  LOG(wait_ret);
  LOG(info.si_status);
  LOG(info.si_code);
  LOG(CLD_EXITED);
}
