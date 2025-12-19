#include <ivl/build_system/unshare>

// IVL add_compiler_flags("-static -flto -Wno-write-strings")

int main() {
  namespace sys = ivl::terminate_syscalls;
  ivl::isolate();

  auto ro_bind = [](const char* in, const char* out) {
    sys::close(sys::creat(out, 0755));
    sys::mount((char*)in, (char*)out, nullptr, MS_BIND, nullptr);
    sys::mount(nullptr, (char*)out, nullptr, MS_REMOUNT | MS_BIND | MS_RDONLY, nullptr);
  };

  sys::mount("tmpfs", "/dev/shm/mount-point", "tmpfs", 0, (void*)"size=1G");
  sys::mkdir("/dev/shm/mount-point/libs", 0755);
  sys::mkdir("/dev/shm/mount-point/lib64", 0755);

  ro_bind("/usr/bin/cat", "/dev/shm/mount-point/cat");
  ro_bind("/usr/bin/ls", "/dev/shm/mount-point/ls");
  ro_bind("/usr/bin/stat", "/dev/shm/mount-point/stat");
  ro_bind("/home/ilazaric/repos/ALL/ivl/build_system/foo", "/dev/shm/mount-point/foo");
  ro_bind("/lib/x86_64-linux-gnu/libselinux.so.1", "/dev/shm/mount-point/libs/libselinux.so.1");
  ro_bind("/lib/x86_64-linux-gnu/libc.so.6", "/dev/shm/mount-point/libs/libc.so.6");
  ro_bind("/lib/x86_64-linux-gnu/libpcre2-8.so.0", "/dev/shm/mount-point/libs/libpcre2-8.so.0");
  ro_bind("/usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2", "/dev/shm/mount-point/lib64/ld-linux-x86-64.so.2");

  {
    auto pid = sys::fork();
    if (pid == 0) {
      sys::chroot("/dev/shm/mount-point");
      const char* argv[] = {"/ls", "/libs", nullptr};
      const char* envp[] = {"LD_LIBRARY_PATH=/libs", nullptr};
      sys::execve(argv[0], argv, envp);
    }
    sys::wait4(pid, nullptr, 0, nullptr);
  }

  {
    auto pid = sys::fork();
    if (pid == 0) {
      sys::chroot("/dev/shm/mount-point");
      const char* argv[] = {"/ls", "-lah", "/foo", nullptr};
      const char* envp[] = {"LD_LIBRARY_PATH=/libs", nullptr};
      sys::execve(argv[0], argv, envp);
    }
    sys::wait4(pid, nullptr, 0, nullptr);
  }

  {
    auto pid = sys::fork();
    if (pid == 0) {
      sys::chroot("/dev/shm/mount-point");
      // sys::symlink("/foo", "/bar");
      // auto fd = sys::open("/bar", O_RDWR, 0);
      // sys::dup2(fd, 1);
      // sys::close(fd);
      const char* argv[] = {"/stat", "/foo", nullptr};
      const char* envp[] = {"LD_LIBRARY_PATH=/libs", nullptr};
      sys::execve(argv[0], argv, envp);
    }
    sys::wait4(pid, nullptr, 0, nullptr);
  }
}
