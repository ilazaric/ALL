#include <ivl/linux/terminate_syscalls>

namespace sys = ivl::linux::terminate_syscalls;

int main() {

  LOG(sys::fcntl(0, F_GETFD, 0));
  LOG(sys::fcntl(1, F_GETFD, 0));
  LOG(sys::fcntl(2, F_GETFD, 0));

  // dup() kills CLOEXEC
  // auto cflags = O_RDONLY | O_CLOEXEC;
  // auto fd = sys::open("/etc/adduser.conf", cflags, 0);
  // auto flags = sys::fcntl(fd, F_GETFD, 0);
  // LOG(fd, O_CLOEXEC, FD_CLOEXEC, cflags, flags);
  // fd = sys::dup(fd);
  // flags = sys::fcntl(fd, F_GETFD, 0);
  // LOG(fd, O_CLOEXEC, FD_CLOEXEC, cflags, flags);

  // EPERM from munmap
  // auto fd = sys::open("/etc/adduser.conf", cflags, 0);
  // auto ptr = sys::mmap(0, 1, PROT_READ, MAP_PRIVATE, fd, 0);
  // sys::mseal(ptr, 1, 0);
  // sys::munmap(ptr, 1);

  // persistence, doesn't work for cgroup dirs though
  // LOG(sys::mkdir("foo", 0755));
  // auto fd = sys::open("foo", O_DIRECTORY | O_RDONLY, 0);
  // LOG(sys::rmdir("foo"));
}
