#include <ivl/linux/terminate_syscalls>
#include <cassert>

// IVL add_compiler_flags("-static")

int main() {
  namespace sys = ivl::linux::terminate_syscalls;

  char pathname[] = "000";
  for (int i = 0; i < 1000; ++i) {
    pathname[0] = '0' + i / 100;
    pathname[1] = '0' + i / 10 % 10;
    pathname[2] = '0' + i % 10;
    auto fd = sys::openat(3, pathname, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    char data[1024]{};
    for (int j = 0; j < 1024; ++j)
      assert(1024 == sys::write(fd, data, sizeof(data)));
    sys::close(fd);
  }
}
