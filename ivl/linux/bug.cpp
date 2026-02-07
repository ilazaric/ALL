// IVL add_compiler_flags("-Wl,-z,noseparate-code -flto -static -nolibc -nostartfiles -fno-stack-protector")
#pragma IVL add_compiler_flags -Wl,-z,noseparate-code -flto -static -nolibc -nostartfiles -fno-stack-protector

extern "C" __attribute__((used)) void actual_start() {
  long a          = 0;
  long b          = 0;
  auto do_compute = [&a, &b] {};
  asm volatile("" : : "g"(&do_compute) : "memory");
}

extern "C" __attribute__((naked)) [[noreturn]] void _start() {
  asm volatile("call actual_start\n"
               "ud2\n"
               :
               :
               : "memory");
  __builtin_unreachable();
}
