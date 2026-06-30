#include <ivl/linux/throwing_syscalls>
#include <ivl/linux/utility>
#include "replicate_standard_filesystem"

int ivl_main() {
  namespace sys = ivl::linux::throwing_syscalls;
  sys::unshare(CLONE_NEWNS | CLONE_NEWUSER | CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWCGROUP);
  ivl::linux::write_file_slow("/proc/self/uid_map", "0 1000 1");
  ivl::linux::write_file_slow("/proc/self/setgroups", "deny");
  ivl::linux::write_file_slow("/proc/self/gid_map", "0 1000 1");
  ivl::isolated_execution::replicate_standard_filesystem("/tmp");
  return 0;
}
