#include <ivl/linux/terminate_syscalls>
#include <span>
#include <ivl/command_line_argument_parsing/passthrough>

// IVL add_compiler_flags("-static")

struct args {};

int ivl_main(args, ivl::cmdline_parsing::passthrough cmd) {
  namespace sys = ivl::linux::terminate_syscalls;
  contract_assert(cmd.data.size() >= 3);
  auto ctl = cmd.data[0];
  auto ctl_ack = cmd.data[1];
  cmd.data = cmd.data.subspan(2);
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
  sys::execve(cmd.data[0], &cmd.data[0], nullptr /* TODO */);
  return 0;
}
