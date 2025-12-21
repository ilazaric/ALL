#include <ivl/linux/terminate_syscalls>

namespace sys = ivl::linux::terminate_syscalls;

int fork_exec_wait4() {
  auto pid = sys::fork();
  if (pid == 0) sys::execve("./tiny_exe_exit_1", nullptr, nullptr);
  int wstatus = 0;
  sys::wait4(pid, &wstatus, 0, nullptr);
  return WEXITSTATUS(wstatus);
}

int vfork_exec_wait4() {
  auto pid = ivl::linux::raw_syscalls::manual_syscall((long)ivl::linux::syscall_number::vfork);
  if (pid == 0) sys::execve("./tiny_exe_exit_1", nullptr, nullptr);
  int wstatus = 0;
  sys::wait4(pid, &wstatus, 0, nullptr);
  return WEXITSTATUS(wstatus);
}

int clone3_exec_wait4(bool vfork) {
  alignas(16) char stack[1<<12];
  clone_args args{};
  args.flags = CLONE_CLEAR_SIGHAND | CLONE_FILES | CLONE_FS | CLONE_IO | CLONE_VM | (vfork ? CLONE_VFORK : 0);
  args.stack = reinterpret_cast<uintptr_t>(&stack[0]);
  args.stack_size = sizeof(stack);
  args.exit_signal = SIGCHLD;
  auto pid = ivl::linux::raw_syscalls::fat_clone3(
    &args, sizeof(args), nullptr, +[](void*) noexcept { sys::execve("./tiny_exe_exit_1", nullptr, nullptr); }
  );
  if (pid <= 0) {
    LOG(-pid);
    sys::exit_group(1);
  }
  int wstatus = 0;
  sys::wait4(pid, &wstatus, 0, nullptr);
  return WEXITSTATUS(wstatus);
}

int main() {
  int count = 1'000'0;
  int sum = 0;
  for (int i = 0; i < count; ++i) sum += vfork_exec_wait4();
  LOG(sum);
}
