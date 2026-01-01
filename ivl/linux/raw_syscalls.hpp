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
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/prctl.h>
#include <linux/landlock.h>
// these are not provided
typedef unsigned short umode_t;
typedef uid_t qid_t;
typedef __kernel_rwf_t rwf_t;
typedef int32_t key_serial_t;
// :(
#define __old_kernel_stat stat

struct __aio_sigset;
struct cachestat;
struct cachestat_range;
struct epoll_event;
struct file_handle;
struct futex_waitv;
struct getcpu_cache;
struct iocb;
struct io_event;
struct io_uring_params;
struct iovec;
struct __kernel_itimerspec;
struct __kernel_old_itimerval;
struct __kernel_old_timeval;
struct __kernel_timespec;
struct __kernel_timex;
struct kexec_segment;
struct landlock_ruleset_attr;
struct linux_dirent;
struct linux_dirent64;
struct lsm_ctx;
struct mmsghdr;
struct mnt_id_req;
struct mount_attr;
struct mq_attr;
struct msgbuf;
struct msqid_ds;
struct __old_kernel_stat;
struct old_utsname;
struct open_how;
struct perf_event_attr;
struct pollfd;
struct rlimit;
struct rlimit64;
struct robust_list_head;
struct rseq;
struct rusage;
struct sched_attr;
struct sched_param;
struct sembuf;
struct shmid_ds;
struct sigaction;
struct sigevent;
struct siginfo;
struct sockaddr;
struct stat;
struct statfs;
struct statmount;
struct statx;
struct sysinfo;
struct timezone;
struct tms;
struct user_msghdr;
struct ustat;
struct utimbuf;
struct xattr_args;
union bpf_attr;

// struct cachestat_range;
// struct cachestat;

#include <linux/sched.h> /* Definition of struct clone_args */
#include <sched.h>       /* Definition of CLONE_* constants */
#include <linux/io_uring.h>

/*
  In linux repository you will find syscalls defined via macros.
  SYSCALL_DEFINEn(name, ...) for n <= 6
  This is the ultimate source of truth on number of parameters and their types.
  Note some syscalls are arch specific.
  There are also nice tables describing the syscall numbers, like:
  arch/x86/entry/syscalls/syscall_64.tbl

  linux repo was parsed, headers syscall_numbers_X and syscall_arguments_X were generated.

  This header only cares about x86-64.
 */

#include <ivl/linux/syscall_numbers>

// TODO: clang-format should indent the error
#if !defined(__x86_64__)
#error "This header only works for x86-64"
#endif

#if !defined(__linux__)
#error "This header only works for linux"
#endif

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

  inline long manual_syscall(long nr) {
    register long rax asm("rax") = nr;
    asm volatile("syscall"
                 : "+a"(rax) /* outputs */
                 :           /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  inline long manual_syscall(long nr, long arg0) {
    register long rax asm("rax") = nr;
    register long rdi asm("rdi") = arg0;
    asm volatile("syscall"
                 : "+a"(rax) /* outputs */
                 : "r"(rdi)  /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  inline long manual_syscall(long nr, long arg0, long arg1) {
    register long rax asm("rax") = nr;
    register long rdi asm("rdi") = arg0;
    register long rsi asm("rsi") = arg1;
    asm volatile("syscall"
                 : "+a"(rax)          /* outputs */
                 : "r"(rdi), "r"(rsi) /* inputs */
                 : "memory", "rcx", "r11", "cc" /* clobbers */);
    return rax;
  }

  inline long manual_syscall(long nr, long arg0, long arg1, long arg2) {
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

  inline long manual_syscall(long nr, long arg0, long arg1, long arg2, long arg3) {
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

  inline long manual_syscall(long nr, long arg0, long arg1, long arg2, long arg3, long arg4) {
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

  inline long manual_syscall(long nr, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5) {
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

  inline long argument_convert(auto* ptr) { return reinterpret_cast<long>(ptr); }
  inline long argument_convert(long num) { return num; }

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
  inline long name(X_PARAMS##N(__VA_ARGS__)) {                                                                         \
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

  // some syscalls were stripped out of syscall_arguments_X due to complex semantics
  // they are: vfork, clone, clone3
  // they have been moved to syscall_arguments_controlflow_X

  inline long fat_clone3(const clone_args* args, size_t size, void* fnarg, void (*fn)(void*) noexcept) {
    register long rax asm("rax") = (long)syscall_number::clone3;
    register long rdi asm("rdi") = reinterpret_cast<long>(args);
    register long rsi asm("rsi") = static_cast<long>(size);
    register long rdx asm("rdx") = reinterpret_cast<long>(fn);
    register long r10 asm("r10") = reinterpret_cast<long>(fnarg);
    asm volatile goto("syscall\n\t"
                      "test %%rax, %%rax\n\t"
                      "jnz %l[parent_process]\n\t"
                      "mov %%r10, %%rdi\n\t"
                      "call *%%rdx"
                      : "+a"(rax)                              /* outputs */
                      : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10) /* inputs */
                      : "memory", "rcx", "r11", "cc"           /* clobbers */
                      : parent_process /* goto labels */);

  parent_process:
    return rax;
  }

} // namespace ivl::linux::raw_syscalls
