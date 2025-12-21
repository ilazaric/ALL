#include <ivl/build_system/unshare>
#include <cassert>
#include <filesystem>
#include <vector>
#include <sys/prctl.h>
#include <sys/personality.h>

// IVL add_compiler_flags("-static -flto -Wno-write-strings")

namespace sys = ivl::terminate_syscalls;

void ro_bind(const std::filesystem::path& in, const std::filesystem::path& out) {
  // std::cerr << in << " -> " << out << std::endl;
  create_directories(out.parent_path());
  sys::close(sys::creat(out.c_str(), 0755));
  sys::mount((char*)in.c_str(), (char*)out.c_str(), nullptr, MS_BIND, nullptr);
  sys::mount(nullptr, (char*)out.c_str(), nullptr, MS_REMOUNT | MS_BIND | MS_RDONLY, nullptr);
}

void replicate(const std::filesystem::path& root, std::filesystem::path file) {
  file = absolute(file);
  assert(!is_directory(file));
  while (is_symlink(file)) {
    ro_bind(file, root / file.relative_path());
    file = file.parent_path() / read_symlink(file);
  }
  // std::cerr << "!!! " << file << std::endl;
  assert(is_regular_file(file));
  ro_bind(file, root / file.relative_path());
}

void replicate(const std::filesystem::path& root, const std::vector<std::filesystem::path>& files) {
  for (auto&& file : files) replicate(root, file);
}

int main() {
  // sys::mount("tmpfs", "/home/ilazaric/repos/ALL/ivl/build_system/foo", "tmpfs", 0, (void*)"size=1G");
  // sys::umount2("/home/ilazaric/repos/ALL/ivl/build_system/foo", 0);
  // return 44;

  ivl::isolate();

  std::filesystem::path root = "/dev/shm/mount-point";
  sys::mount("tmpfs", (char*)root.c_str(), "tmpfs", 0, (void*)"size=1G");

  replicate(
    root, std::vector<std::filesystem::path>{
            // "/usr/include/asm-generic/errno-base.h",
            // "/usr/include/asm-generic/errno.h",

            // "/etc/ld.so.cache",

            "/lib/x86_64-linux-gnu/libbfd-2.42-system.so",
            "/lib/x86_64-linux-gnu/libc.so.6",
            "/lib/x86_64-linux-gnu/libctf.so.0",
            "/lib/x86_64-linux-gnu/libgmp.so.10",
            "/lib/x86_64-linux-gnu/libisl.so.23",
            "/lib/x86_64-linux-gnu/libjansson.so.4",
            "/lib/x86_64-linux-gnu/libmpc.so.3",
            "/lib/x86_64-linux-gnu/libmpfr.so.6",
            "/lib/x86_64-linux-gnu/libm.so.6",
            "/lib/x86_64-linux-gnu/libsframe.so.1",
            "/lib/x86_64-linux-gnu/libz.so.1",
            "/lib/x86_64-linux-gnu/libzstd.so.1",
            "tiny.cpp",
            "/usr/bin/as",
            "/usr/bin/g++",
            "/usr/bin/ld",
            // "/usr/include/stdc-predef.h",
            "/usr/libexec/gcc/x86_64-linux-gnu/13/cc1plus",
            "/usr/libexec/gcc/x86_64-linux-gnu/13/collect2",
            "/usr/libexec/gcc/x86_64-linux-gnu/13/liblto_plugin.so",
            "/usr/libexec/gcc/x86_64-linux-gnu/13/lto1",
            "/usr/libexec/gcc/x86_64-linux-gnu/13/lto-wrapper",
            "/usr/lib/gcc/x86_64-linux-gnu/13/crtbeginT.o",
            "/usr/lib/gcc/x86_64-linux-gnu/13/crtend.o",
            "/usr/lib/gcc/x86_64-linux-gnu/13/libgcc.a",
            "/usr/lib/gcc/x86_64-linux-gnu/13/libgcc_eh.a",
            "/usr/lib/gcc/x86_64-linux-gnu/13/libstdc++.a",
            "/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/crt1.o",
            "/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/crti.o",
            "/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/crtn.o",
            "/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/libc.a",
            "/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/libm.a",
            "/usr/lib/x86_64-linux-gnu/libm-2.39.a",
            "/usr/lib/x86_64-linux-gnu/libmvec.a",

            // LC_ALL=C makes these unnecessary
            // "/usr/lib/locale/locale-archive",
            // "/usr/lib/x86_64-linux-gnu/gconv/gconv-modules.cache",
            // "/usr/share/locale/locale.alias",
          }
  );

  replicate(root, "/lib64/ld-linux-x86-64.so.2");

  "/usr/lib/gcc/x86_64-linux-gnu/13/include";
  "/usr/local/include";
#define X(x)                                                                                                           \
  create_directories(root / x);                                                                                        \
  sys::mount("/" x, (char*)(root / x).c_str(), nullptr, MS_BIND, nullptr);                                             \
  sys::mount(nullptr, (char*)(root / x).c_str(), nullptr, MS_REMOUNT | MS_BIND | MS_RDONLY, nullptr);
  X("usr/include");
  X("usr/local/include");
  X("usr/lib/gcc/x86_64-linux-gnu/13/include");
#undef X

  // "/usr/include/c++/13";
  // "/usr/include/x86_64-linux-gnu";

  sys::mkdir((root / "tmp").c_str(), 0755);

  {
    auto pid = sys::fork();
    if (pid == 0) {
      sys::personality(ADDR_NO_RANDOMIZE);
      sys::prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      sys::chroot(root.c_str());
      sys::chdir("/home/ilazaric/repos/ALL/ivl/build_system");
      const char* argv[] = {"/usr/bin/g++", "-static", "-frandom-seed=42", "-flto", "tiny.cpp", nullptr};
      const char* envp[] = {"PATH=/usr/bin", "LC_ALL=C", nullptr};
      sys::execve(argv[0], argv, envp);
    }
    int wstatus;
    sys::wait4(pid, &wstatus, 0, nullptr);
    assert(WIFEXITED(wstatus));
    LOG(WEXITSTATUS(wstatus));
  }

  sys::chroot(root.c_str());
  sys::chdir("/home/ilazaric/repos/ALL/ivl/build_system");
  sys::personality(ADDR_NO_RANDOMIZE);
  sys::execve("a.out", nullptr, nullptr);

  // sys::chroot(root.c_str());

  // sys::mkdir((root / "libs").c_str(), 0755);
  // sys::mkdir((root / "lib64").c_str(), 0755);

  // ro_bind("/usr/bin/cat", root / "cat");
  // ro_bind("/usr/bin/ls", root / "ls");
  // ro_bind("/usr/bin/stat", root / "stat");
  // // ro_bind("/home/ilazaric/repos/ALL/ivl/build_system/foo", root / "foo");
  // ro_bind("/lib/x86_64-linux-gnu/libselinux.so.1", root / "libs/libselinux.so.1");
  // ro_bind("/lib/x86_64-linux-gnu/libc.so.6", root / "libs/libc.so.6");
  // ro_bind("/lib/x86_64-linux-gnu/libpcre2-8.so.0", root / "libs/libpcre2-8.so.0");
  // ro_bind("/usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2", root / "lib64/ld-linux-x86-64.so.2");

  // {
  //   auto pid = sys::fork();
  //   if (pid == 0) {
  //     sys::chroot(root.c_str());
  //     const char* argv[] = {"/ls", "/libs", nullptr};
  //     const char* envp[] = {"LD_LIBRARY_PATH=/libs", nullptr};
  //     sys::execve(argv[0], argv, envp);
  //   }
  //   sys::wait4(pid, nullptr, 0, nullptr);
  // }

  // {
  //   auto pid = sys::fork();
  //   if (pid == 0) {
  //     sys::chroot(root.c_str());
  //     const char* argv[] = {"/ls", "-lah", "/cat", nullptr};
  //     const char* envp[] = {"LD_LIBRARY_PATH=/libs", nullptr};
  //     sys::execve(argv[0], argv, envp);
  //   }
  //   sys::wait4(pid, nullptr, 0, nullptr);
  // }

  // {
  //   auto pid = sys::fork();
  //   if (pid == 0) {
  //     sys::chroot(root.c_str());
  //     // sys::symlink("/foo", "/bar");
  //     // auto fd = sys::open("/bar", O_RDWR, 0);
  //     // sys::dup2(fd, 1);
  //     // sys::close(fd);
  //     const char* argv[] = {"/stat", "/cat", nullptr};
  //     const char* envp[] = {"LD_LIBRARY_PATH=/libs", nullptr};
  //     sys::execve(argv[0], argv, envp);
  //   }
  //   sys::wait4(pid, nullptr, 0, nullptr);
  // }
}
