#include <ivl/build_system/safe_run>
#include <ivl/linux/utils>
#include <ivl/logger>

// IVL add_compiler_flags("-static")

int main(int argc, char* argv[], char* envp[]) {
  namespace sys = ivl::linux::terminate_syscalls;
  using namespace ivl::linux;

  if (sys::getuid() == 0) {
    sys::setgid(1000);
    sys::setuid(1000);
    sys::execve(argv[0], argv, envp);
  }

  auto fd = ivl::linux::create_tmpfs();
  LOG(fd.get());
  assert(fd.get() == 3);
  struct stat statbuf;
  sys::fstat(fd.get(), &statbuf);
  LOG(statbuf.st_mode);
  LOG(statbuf.st_uid);

  ivl::process_config pc;
  pc.pathname = "/home/ilazaric/repos/ALL/ivl/linux/write_bunch";
  auto ret = ivl::safe_run(std::move(pc), {pc.pathname}, {}, std::filesystem::current_path(), 900ULL << 20, 100);
  LOG(ret.wstatus);
}
