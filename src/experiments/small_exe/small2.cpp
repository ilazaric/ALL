// IVL add_compiler_flags("-Wl,-z,noseparate-code -fno-exceptions -fno-rtti -nostdlib -static -nolibc -nostartfiles -fno-stack-protector -c")

#pragma GCC optimize ("align-functions=8")

extern "C"
__attribute__((naked))
void _start() {
  register long rax asm("rax") = 231;
  register long rdi asm("rdi") = 0;
  asm volatile("syscall"
               : "+a"(rax) /* outputs */
               : "r"(rdi)  /* inputs */
               : "memory", "rcx", "r11", "cc" /* clobbers */);
  __builtin_unreachable();
}
