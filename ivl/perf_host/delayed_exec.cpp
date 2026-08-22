#include <ivl/linux/terminate_syscalls>
#include <span>
#include <ivl/command_line_argument_parsing/raw_arguments>

// IVL add_compiler_flags("-static")

struct args {};

int ivl_main(args, ivl::cmdline_parsing::raw_arguments cmd) {
  namespace sys = ivl::linux::terminate_syscalls;
  contract_assert(cmd.size() >= 3);
  auto ctl = cmd[0];
  auto ctl_ack = cmd[1];
  cmd.remove_prefix(2);
  auto ctl_fd = sys::open(ctl, O_WRONLY | O_CLOEXEC, 0);
  auto ctl_ack_fd = sys::open(ctl_ack, O_RDONLY | O_CLOEXEC, 0);

  {
    auto data = "enable\n";
    int len = 7;
    while (*data) {
      int curr = sys::write(ctl_fd, data, len);
      len -= curr;
      data += curr;
    }
  }

  char out[5]{};
  auto len = sys::read(ctl_ack_fd, &out[0], 4);
  contract_assert(len == 4); // "ack\n"
  // printf("%s\n", out);
  sys::execve(cmd.rest[0], &cmd.rest[0], nullptr /* TODO */);
  return 0;
}
