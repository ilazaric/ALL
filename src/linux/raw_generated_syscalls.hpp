#pragma once

// these are needed for all the typedefs
#include <cstdint>
#include <fcntl.h>
#include <linux/aio_abi.h>
#include <linux/capability.h>
#include <linux/fs.h>
#include <linux/landlock.h>
#include <mqueue.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
// these are not provided
typedef unsigned short umode_t;
typedef uid_t qid_t;
typedef __kernel_rwf_t rwf_t;
typedef int32_t key_serial_t;

#include <ivl/linux/syscall_numbers>

namespace ivl::linux::raw_syscalls {

  long manual_syscall(long nr) {
    register long rax asm("rax") = nr;
    asm volatile("syscall"
                 : "+a"(rax) /* outputs */
                 :           /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  long manual_syscall(long nr, long arg0) {
    register long rax asm("rax") = nr;
    register long rdi asm("rdi") = arg0;
    asm volatile("syscall"
                 : "+a"(rax) /* outputs */
                 : "r"(rdi)  /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  long manual_syscall(long nr, long arg0, long arg1) {
    register long rax asm("rax") = nr;
    register long rdi asm("rdi") = arg0;
    register long rsi asm("rsi") = arg1;
    asm volatile("syscall"
                 : "+a"(rax)          /* outputs */
                 : "r"(rdi), "r"(rsi) /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  long manual_syscall(long nr, long arg0, long arg1, long arg2) {
    register long rax asm("rax") = nr;
    register long rdi asm("rdi") = arg0;
    register long rsi asm("rsi") = arg1;
    register long rdx asm("rdx") = arg2;
    asm volatile("syscall"
                 : "+a"(rax)                    /* outputs */
                 : "r"(rdi), "r"(rsi), "r"(rdx) /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  long manual_syscall(long nr, long arg0, long arg1, long arg2, long arg3) {
    register long rax asm("rax") = nr;
    register long rdi asm("rdi") = arg0;
    register long rsi asm("rsi") = arg1;
    register long rdx asm("rdx") = arg2;
    register long r10 asm("r10") = arg3;
    asm volatile("syscall"
                 : "+a"(rax)                              /* outputs */
                 : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10) /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  long manual_syscall(long nr, long arg0, long arg1, long arg2, long arg3, long arg4) {
    register long rax asm("rax") = nr;
    register long rdi asm("rdi") = arg0;
    register long rsi asm("rsi") = arg1;
    register long rdx asm("rdx") = arg2;
    register long r10 asm("r10") = arg3;
    register long r8 asm("r8") = arg4;
    asm volatile("syscall"
                 : "+a"(rax)                                       /* outputs */
                 : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10), "r"(r8) /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  long manual_syscall(long nr, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5) {
    register long rax asm("rax") = nr;
    register long rdi asm("rdi") = arg0;
    register long rsi asm("rsi") = arg1;
    register long rdx asm("rdx") = arg2;
    register long r10 asm("r10") = arg3;
    register long r8 asm("r8") = arg4;
    register long r9 asm("r9") = arg5;
    asm volatile("syscall"
                 : "+a"(rax)                                                /* outputs */
                 : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10), "r"(r8), "r"(r9) /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  long argument_convert(auto* ptr) { return reinterpret_cast<long>(ptr); }
  long argument_convert(long num) { return num; }

#define X_PARAMS0()
#define X_PARAMS1(t1, a1) t1 a1
#define X_PARAMS2(t1, a1, ...) t1 a1, X_PARAMS1(__VA_ARGS__)
#define X_PARAMS3(t1, a1, ...) t1 a1, X_PARAMS2(__VA_ARGS__)
#define X_PARAMS4(t1, a1, ...) t1 a1, X_PARAMS3(__VA_ARGS__)
#define X_PARAMS5(t1, a1, ...) t1 a1, X_PARAMS4(__VA_ARGS__)
#define X_PARAMS6(t1, a1, ...) t1 a1, X_PARAMS5(__VA_ARGS__)

#define X_CARGS0()
#define X_CARGS1(t1, a1) argument_convert(a1)
#define X_CARGS2(t1, a1, ...) argument_convert(a1), X_CARGS1(__VA_ARGS__)
#define X_CARGS3(t1, a1, ...) argument_convert(a1), X_CARGS2(__VA_ARGS__)
#define X_CARGS4(t1, a1, ...) argument_convert(a1), X_CARGS3(__VA_ARGS__)
#define X_CARGS5(t1, a1, ...) argument_convert(a1), X_CARGS4(__VA_ARGS__)
#define X_CARGS6(t1, a1, ...) argument_convert(a1), X_CARGS5(__VA_ARGS__)

#define X(N, name, ...)                                                                                                \
  long name(X_PARAMS##N(__VA_ARGS__)) {                                                                                \
    return manual_syscall((long)::ivl::linux::syscall_number::name __VA_OPT__(, ) X_CARGS##N(__VA_ARGS__));            \
  }
#include <ivl/linux/syscall_arguments_X>

#undef X_PARAMS0
#undef X_PARAMS1
#undef X_PARAMS2
#undef X_PARAMS3
#undef X_PARAMS4
#undef X_PARAMS5
#undef X_PARAMS6

#undef X_CARGS0
#undef X_CARGS1
#undef X_CARGS2
#undef X_CARGS3
#undef X_CARGS4
#undef X_CARGS5
#undef X_CARGS6

  // We add `unavailable` attribute to make previously generated functions unusable.
  // The corresponding syscall semantics are too complex to be used as regular functions.
#define X_FORBID(name) decltype(name) __attribute__((unavailable)) name
  X_FORBID(fork);
  X_FORBID(vfork);
  X_FORBID(clone);
  X_FORBID(clone3);
#undef X_FORBID

} // namespace ivl::linux::raw_syscalls
