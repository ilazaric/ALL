#include <ivl/linux/file_descriptor>
#include <ivl/linux/kernel_result>
#include <ivl/linux/raw_syscalls>
#include <ivl/linux/typed_syscalls>

#include <fcntl.h>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>
#include <x86intrin.h> // __rdtsc, __rdtscp, __cpuid

// IVL ADD_COMPILER_FLAGS -Wl,-z,noseparate-code -flto -static -nolibc -nostartfiles -fno-stack-protector

// constexpr char msg[] = "Hello world!\n";

// struct S {
//   S(){}
//   ~S(){}
// };

// S s;

int main(int argc, char** argv) {
  int num;
  sscanf(argv[1], "%d", &num);
  auto start = __rdtsc();
  long sv    = 12;
  long cv    = sv;
  for (int i = 0; i < num; ++i)
    cv = ivl::linux::raw_syscalls::inc(cv);
  auto end = __rdtsc();
  std::cout << "rep count: " << num << std::endl;
  std::cout << "cycle count: " << end - start << std::endl;
  std::cout << "avg cycle count: " << (end - start) * 1.0 / num << std::endl;
  std::cout << "correct: " << (cv == sv + num) << std::endl;
}

#if 0
// int main() {
extern "C" [[noreturn]] void _start() {
  // auto wc    = [](char c) { ivl::linux::raw_syscalls::write(1, &c, 1); };
  // auto print = [&](uint64_t x) {
  //   for (int i = 60; i >= 0; i -= 4) {
  //     int val = (x >> i) & 0xF;
  //     wc(val <= 9 ? val + '0' : val - 10 + 'A');
  //   }
  //   wc('\n');
  // };
  // auto print2 = [&](uint64_t x) {
  //   print(x);
  //   print(-x);
  //   wc('\n');
  // };
  // auto ret = (uint64_t)ivl::linux::raw_syscalls::inc(3);
  // print(ret);
  // print(-ret);
  long x = 3;
  // for (int i = 0; i < 1; ++i) {
  // wc(x);
  ivl::linux::raw_syscalls::write(1, (const char*)&x, 1);
  // x = ivl::linux::raw_syscalls::inc(x);
  // }
  ivl::linux::raw_syscalls::exit_group(0);
  //   auto* ptr       = &msg[0];
  //   auto  rem       = sizeof(msg) - 1;
  //   int   exit_code = 0;
  //   {
  //     auto fd = ivl::linux::open("dump", O_WRONLY | O_CREAT, 0644);
  //     if (fd.is_error()) {
  //       exit_code = 1;
  //       goto end;
  //     }
  //     while (rem) {
  //       fd.with_success([&](const ivl::linux::file_descriptor fd) {
  //         auto written = ivl::linux::raw_syscalls::write(fd.get(), ptr, rem);
  //         if (written < 0) {
  //           exit_code = -written;
  //           rem       = 0;
  //         } else rem -= written;
  //       });
  //     }
  //   }
  // end:
  //   ivl::linux::raw_syscalls::exit_group(exit_code);
  std::unreachable();
}
#endif
