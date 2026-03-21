#include <ivl/linux/terminate_syscalls>
#include <ivl/linux/throwing_syscalls>

int main() {
  namespace sys = ivl::linux::throwing_syscalls;
  sys::close_range(3, std::numeric_limits<int>::max(), 0);
  auto pid = sys::fork();
  if (pid == 0) {
    struct timespec ts{.tv_sec = 1};
    while (true) sys::nanosleep(&ts, nullptr);
  }

  
}
