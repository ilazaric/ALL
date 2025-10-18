#include <ivl/linux/file_descriptor>
#include <ivl/linux/raw_syscalls>
#include <ivl/linux/rich_syscalls>
#include <ivl/linux/throwing_syscalls>
#include <ivl/linux/typed_syscalls>
#include <ivl/process>
#include <cassert>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>

// IVL add_compiler_flags("-static -flto")

ivl::linux::or_syscall_error<ivl::linux::owned_file_descriptor> compile_cxx(std::string_view code) {
  auto in_fd  = ivl::linux::rich::open("/dev/shm", O_TMPFILE | O_RDWR, 0777).unwrap_or_throw();
  auto out_fd = ivl::linux::rich::open("/dev/shm", O_TMPFILE | O_RDWR, 0777).unwrap_or_throw();

  {
    auto rem = code;
    while (!rem.empty()) {
      auto ret = ivl::linux::raw_syscalls::write(in_fd.get(), rem.data(), rem.size());
      if (ret < 0) {
        std::cout << "write: " << ret << std::endl;
        exit(1);
      }
      rem.remove_prefix(ret);
    }
  }

  ivl::process_config cfg;
  cfg.pathname = "/usr/bin/g++";
  cfg.argv     = {"/usr/bin/g++", "-xc++", "-std=c++23", "-O3", "-static", "-o", "/dev/stdout", "-"};
  // needs PATH to find linker
  cfg.envp           = {{"PATH", "/usr/bin"}};
  cfg.pre_exec_setup = [&] {
    assert(ivl::linux::raw_syscalls::dup2(in_fd.get(), 0) >= 0);
    // wouldnt be necessary had i used pwrite
    assert(ivl::linux::raw_syscalls::lseek(0, 0, SEEK_SET) >= 0);
    assert(ivl::linux::raw_syscalls::dup2(out_fd.get(), 1) >= 0);
  };

  auto proc = cfg.clone_and_exec().unwrap_or_terminate();
  auto w    = proc.wait();
  assert(w.is_success());
  std::cout << "w: " << w.unwrap_or_terminate() << std::endl;
  assert(w.unwrap_or_terminate() == 0);

  char link[64];
  snprintf(link, sizeof(link), "/proc/self/fd/%d", out_fd.get());
  out_fd = ivl::linux::rich::open(link, O_RDONLY, 0).unwrap_or_throw();

  return ivl::linux::or_syscall_error<ivl::linux::owned_file_descriptor>(std::move(out_fd));
}

int main() {
  auto out_fd = compile_cxx(R"CXX(
#include <iostream>
int main(){
  std::cout << "Hello world" << std::endl;
}
)CXX")
                  .unwrap_or_terminate();

  auto ret = ivl::linux::raw_syscalls::execveat(out_fd.get(), "", nullptr, nullptr, AT_EMPTY_PATH);
  std::cout << "unexpected ret: " << ret << std::endl;
  ivl::linux::raw_syscalls::ud2();

  // ivl::linux::raw_syscalls::fat_clone3();

  // if (fd.is_error()){
  //   std::cout << "ERROR: " << fd.error.get().value << std::endl;
  //   return 1;
  // }

  return 0;
}
