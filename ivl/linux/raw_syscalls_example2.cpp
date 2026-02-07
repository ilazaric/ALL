#include <ivl/linux/raw_syscalls>
#include <utility>

// IVL add_compiler_flags("-Wl,-z,noseparate-code -flto -static -nolibc -nostartfiles -fno-stack-protector -fno-exceptions -fno-rtti")
#pragma IVL add_compiler_flags -Wl,-z,noseparate-code -flto -static -nolibc -nostartfiles -fno-stack-protector -fno-exceptions -fno-rtti


extern "C" __attribute__((naked)) [[noreturn]] void _start() {
  asm volatile("call actual_start\n"
               "ud2\n"
               :
               :
               : "memory");
  __builtin_unreachable();
}

extern "C" __attribute__((used)) [[noreturn]] void actual_start() {
  ivl::linux::raw_syscalls::exit_group(123);
  std::unreachable();
}
