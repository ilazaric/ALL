#include <ivl/linux/raw_syscalls>
#include <ivl/util>

#include <cstdint>
#include <string_view>
#include <utility>

#include <sys/mman.h>

// TODO: figure out why this doesnt get picked up
// extern "C" __attribute__((used)) size_t strlen(const char* str) {
//   size_t ret = 0;
//   while (str[ret])
//     ++ret;
//   return ret;
// }

void print(std::string_view sv) {
  while (!sv.empty()) {
    auto ret = ivl::linux::raw_syscalls::write(1, sv.data(), sv.size());
    if (ret < 0) {
      // TODO: handle error
      break;
    }
    sv.remove_prefix(ret);
  }
}

void print(
  ivl::util::same_as_one_of<
    short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long> auto num
) {
  char               memory[30]; // len(str(2**64)) == 20, should be good enough
  unsigned long long abs = num;
  auto               end = &memory[0] + sizeof(memory);
  auto               wp  = end;
  if (num < 0) abs = -abs;
  do
    *--wp = abs % 10 + '0', abs /= 10;
  while (abs);
  if (num < 0) *--wp = '-';
  print(std::string_view(wp, end));
}

// void print(long num) {
//   char out[30];
//   auto ptr_e = &out[29];
//   auto ptr   = ptr_e;
//   *ptr       = '0';
//   while (num)
//     *(ptr--) = num % 10 + '0', num /= 10;
//   print({ptr + 1, ptr_e + 1});
// }

void print_hex(long num) {
  char out[30];
  auto ptr_e = &out[29];
  auto ptr   = ptr_e;
  *ptr       = '0';
  while (num)
    *(ptr--) = (num % 16 < 10 ? num % 16 + '0' : num % 16 - 10 + 'A'), num /= 16;
  print({ptr + 1, ptr_e + 1});
}

void print_error(std::string_view ctx, long error) {
  print(ctx);
  print(": ");
  print(-error);
  print("\n");
  ivl::linux::raw_syscalls::exit_group(1);
}

namespace ivl {

  // TODO: prctl to kill on parent death
  struct thread {
    char* stack;
    int   child_pid;

    thread() : stack(nullptr), child_pid(0) {}

    thread(const thread&)            = delete;
    thread& operator=(const thread&) = delete;

    thread(thread&& o) : stack(o.stack), child_pid(o.child_pid) {
      o.stack     = nullptr;
      o.child_pid = 0;
    }

    thread& operator=(thread&& o) {
      if (this == &o) return *this;
      // TOOD: swap or kill?
      std::swap(stack, o.stack);
      std::swap(child_pid, o.child_pid);
      return *this;
    }

    explicit thread(auto&& callable) {
      using T  = std::remove_cvref_t<decltype(callable)>;
      auto ret = ivl::linux::raw_syscalls::mmap(
        0, 4ULL << 12, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS // | MAP_GROWSDOWN
        ,
        -1, 0
      );
      if (ret < 0) print_error("mmap", ret);
      stack          = reinterpret_cast<char*>(ret);
      auto stack_end = stack + (4ULL << 12);
      static_assert(sizeof(T) < (1ULL << 12));
      auto moved = new (stack_end - sizeof(T)) T(FWD(callable));

      clone_args args{
        .flags = CLONE_VM | CLONE_FS | CLONE_FILES // | CLONE_THREAD
               | CLONE_SIGHAND | CLONE_IO,
        .pidfd{},
        .child_tid{},
        .parent_tid{},
        .exit_signal = SIGCHLD,
        .stack       = reinterpret_cast<uintptr_t>(stack),
        .stack_size  = 4ULL << 12,
        .tls{},
        .set_tid{},
        .set_tid_size{},
        .cgroup{},
      };
      auto ret2 = ivl::linux::raw_syscalls::clone3(&args, sizeof(args));
      if (ret2 < 0) print_error("clone3", ret);
      if (ret2 > 0) {
        child_pid = ret2;
        return;
      }

      // TODO
      register long rsp asm("rsp");
      (*reinterpret_cast<T*>(rsp))();
      ivl::linux::raw_syscalls::exit(0);
      std::unreachable();
    }

    void join() {
      // TODO
      if (child_pid == 0) return;
      auto ret = ivl::linux::raw_syscalls::wait4(child_pid, nullptr, 0, nullptr);
      if (ret < 0) print_error("wait4", ret);
    }

    ~thread() { join(); }
  };

} // namespace ivl

extern "C" [[noreturn]] void _start() {
  print("Main thread\n");
  {
    ivl::thread th([] { print("Thread :)\n"); });
  }
  // {
  //   alignas(4096) char stack[4096];
  //   clone_args         args{
  //             .flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_VFORK | CLONE_THREAD | CLONE_SIGHAND | CLONE_IO,
  //             .pidfd{},
  //             .child_tid{},
  //             .parent_tid{},
  //             .exit_signal = 0,
  //             .stack       = reinterpret_cast<uintptr_t>(&stack[0]),
  //             .stack_size  = 4096,
  //             .tls{},
  //             .set_tid{},
  //             .set_tid_size{},
  //             .cgroup{},
  //   };
  //   auto ret = ivl::linux::raw_syscalls::clone3(&args, sizeof(args));
  //   if (ret < 0) print_error("clone3", ret);
  //   if (ret == 0) {
  //     print("Child thread\n");
  //     ivl::linux::raw_syscalls::exit(0);
  //   }
  //   print("Main thread again\n");
  // }
  ivl::linux::raw_syscalls::exit_group(0);
  std::unreachable();
}
