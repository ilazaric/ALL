#include <ivl/linux/raw_syscalls>

// IVL add_compiler_flags("-Wl,-z,noseparate-code,--gc-sections")
// IVL add_compiler_flags("-fno-exceptions -fno-rtti")
// IVL add_compiler_flags("-nostdlib -nolibc -nostartfiles -static -fno-stack-protector")
// IVL add_compiler_flags("-fcf-protection=none -fno-asynchronous-unwind-tables")
// IVL add_compiler_flags("-Wl,--build-id=none")
// IVL add_compiler_flags("-Wl,--entry=_start")
// IVL add_compiler_flags("-Wl,-z,nosectionheader")
// IVL add_compiler_flags("-falign-functions=1")
// IVL add_compiler_flags("-falign-jumps=1")
// IVL add_compiler_flags("-falign-labels=1")
// IVL add_compiler_flags("-falign-loops=1")
// IVL add_compiler_flags("-Wl,-z,noexecstack")

__attribute__((aligned(1)))
constexpr char msg[] = "Hello world\n";

extern "C" __attribute__((used)) [[noreturn]] void premain() {
  const char* cursor = msg;
  long remaining = sizeof(msg);
  long exit_code = 0;
  while (remaining) {
    long ret = ivl::linux::raw_syscalls::write(1, cursor, remaining);
    if (ret < 0) {
      exit_code = 1;
      break;
    }
    remaining -= ret;
    cursor += ret;
  }
  ivl::linux::raw_syscalls::exit_group(exit_code);
  __builtin_unreachable();
}

extern "C" __attribute__((naked, used, aligned(1))) [[noreturn]] void _start() {
  asm("call premain\n\t"
      "ud2");
  __builtin_unreachable();
}
