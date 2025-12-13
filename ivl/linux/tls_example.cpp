#include <ivl/linux/raw_syscalls>
#include <utility>

// int main() {
extern "C" [[noreturn]] void _start() {
  long x = 3;
  ivl::linux::raw_syscalls::write(1, (const char*)&x, 1);
  ivl::linux::raw_syscalls::exit_group(0);
  std::unreachable();
}
