#include <ivl/linux/raw_syscalls>

// Basically taken from ivl/experiments/small_exe/small2.cpp.
// As small as it gets, except for the GNU_STACK program header.
// - readelf -e
// This is used in the process_creation_benchmark, no other purpose.
// (other than showcasing the size)

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

extern "C" __attribute__((used)) [[noreturn]] void premain() {
  ivl::linux::raw_syscalls::exit_group(1);
  __builtin_unreachable();
}

extern "C" __attribute__((naked, used, aligned(1))) [[noreturn]] void _start() {
  asm("call premain\n\t"
      "ud2");
  __builtin_unreachable();
}
