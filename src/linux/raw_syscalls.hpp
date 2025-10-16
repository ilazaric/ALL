#pragma once

/*
  In linux repository you will find syscalls defined via macros.
  SYSCALL_DEFINEn(name, ...) for n <= 6
  This is the ultimate source of truth on number of parameters and their types.
  Note some syscalls are arch specific.
  There are also nice tables describing the syscall numbers, like:
  arch/x86/entry/syscalls/syscall_64.tbl

  This header only cares about x86-64.


 */

// TODO: clang-format should indent the error
#if !defined(__x86_64__)
#error "This header only works for x86-64"
#endif

#if !defined(__linux__)
#error "This header only works for linux"
#endif

#include <linux/sched.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <utility>

namespace ivl::linux::raw_syscalls {

  /*
    from linux docs (arch/x86/entry/entry_64.S):

     * 64-bit SYSCALL instruction entry. Up to 6 arguments in registers.
     *
     * This is the only entry point used for 64-bit system calls.  The
     * hardware interface is reasonably well designed and the register to
     * argument mapping Linux uses fits well with the registers that are
     * available when SYSCALL is used.
     *
     * SYSCALL instructions can be found inlined in libc implementations as
     * well as some other programs and libraries.  There are also a handful
     * of SYSCALL instructions in the vDSO used, for example, as a
     * clock_gettimeofday fallback.
     *
     * 64-bit SYSCALL saves rip to rcx, clears rflags.RF, then saves rflags to r11,
     * then loads new ss, cs, and rip from previously programmed MSRs.
     * rflags gets masked by a value from another MSR (so CLD and CLAC
     * are not needed). SYSCALL does not save anything on the stack
     * and does not change rsp.
     *
     * Registers on entry:
     * rax  system call number
     * rcx  return address
     * r11  saved rflags (note: r11 is callee-clobbered register in C ABI)
     * rdi  arg0
     * rsi  arg1
     * rdx  arg2
     * r10  arg3 (needs to be moved to rcx to conform to C ABI)
     * r8   arg4
     * r9   arg5
     * (note: r12-r15, rbp, rbx are callee-preserved in C ABI)
     *
     * Only called from user space.
     *
     * When user can change pt_regs->foo always force IRET. That is because
     * it deals with uncanonical addresses better. SYSRET has trouble
     * with them due to bugs in both AMD and Intel CPUs.
   */

  // TODO: this is kinda stupid, a lot of repetition, figure out something

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
    register long r8 asm("r8")   = arg4;
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
    register long r8 asm("r8")   = arg4;
    register long r9 asm("r9")   = arg5;
    asm volatile("syscall"
                 : "+a"(rax)                                                /* outputs */
                 : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10), "r"(r8), "r"(r9) /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  inline __attribute__((always_inline)) [[noreturn]] void ud2() {
    asm volatile("ud2" : : : "memory");
    std::unreachable();
  }

  // TODO

  // long mmap();

  long read(unsigned int fd, char* buf, size_t count) {
    return manual_syscall(0, fd, reinterpret_cast<long>(buf), count);
  }

  long write(unsigned int fd, const char* buf, size_t count) {
    return manual_syscall(1, fd, reinterpret_cast<long>(buf), count);
  }

  long open(const char* filename, int flags, mode_t mode) {
    return manual_syscall(2, reinterpret_cast<long>(filename), flags, mode);
  }

  long close(unsigned int fd) { return manual_syscall(3, fd); }

  long exit(int error_code) { return manual_syscall(60, error_code); }

  long exit_group(int error_code) { return manual_syscall(231, error_code); }

  long execve(const char* pathname, const char* const argv[], const char* const envp[]) {
    return manual_syscall(
      59, reinterpret_cast<long>(pathname), reinterpret_cast<long>(argv), reinterpret_cast<long>(envp)
    );
  }

  // long clone3(const clone_args* args, size_t size) { return manual_syscall(435, reinterpret_cast<long>(args), size);
  // }

  // TODO: think on this more, clone3 is weird, weird control flow
  long fat_clone3(const clone_args* args, size_t size, void* fnarg, void (*fn)(void*) noexcept) {
    register long rax asm("rax") = 435;
    register long rdi asm("rdi") = reinterpret_cast<long>(args);
    register long rsi asm("rsi") = static_cast<long>(size);
    register long rdx asm("rdx") = reinterpret_cast<long>(fn);
    register long r10 asm("r10") = reinterpret_cast<long>(fnarg);
    asm volatile goto("syscall\n"
                      "test %%rax, %%rax\n"
                      "jnz %l[parent_process]\n"
                      "mov %%r10, %%rdi\n"
                      "call *%%rdx\n"
                      : "+a"(rax)                              /* outputs */
                      : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10) /* inputs */
                      : "memory", "rcx", "r11", "cc"           /* clobbers */
                      : parent_process /* goto labels */);

    // reinterpret_cast<void (*)(void*)>(rdx)(reinterpret_cast<void*>(r10));

  parent_process:
    return rax;
  }

  long dup2(unsigned int oldfd, unsigned int newfd) { return manual_syscall(33, oldfd, newfd); }

  long lseek(unsigned int fd, off_t offset, unsigned int whence) { return manual_syscall(8, fd, offset, whence); }

  // long fatter_clone3(const clone_args* args);

  long mmap(
    unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, unsigned long fd, unsigned long off
  ) {
    return manual_syscall(9, addr, len, prot, flags, fd, off);
  }

  long wait4(pid_t upid, int* stat_addr, int options, rusage* ru) {
    return manual_syscall(61, upid, reinterpret_cast<long>(stat_addr), options, reinterpret_cast<long>(ru));
  }

  long ftruncate(unsigned int fd, off_t length) { return manual_syscall(77, fd, length); }

  long execveat(int fd, const char* filename, const char* const* argv, const char* const* envp, int flags) {
    return manual_syscall(
      322, fd, reinterpret_cast<long>(filename), reinterpret_cast<long>(argv), reinterpret_cast<long>(envp), flags
    );
  }

  long inc(long arg) { return manual_syscall(666, arg); }

} // namespace ivl::linux::raw_syscalls
