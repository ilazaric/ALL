#include <ivl/linux/raw_syscalls>
#include <ivl/meta>
#include <ivl/util>

#include <atomic>
#include <cstdint>
#include <string_view>
#include <utility>
#include <x86intrin.h> // __rdtsc, __rdtscp, __cpuid

#include <sys/mman.h>

// IVL add_compiler_flags("-Wl,-z,noseparate-code -flto -static -nolibc -nostartfiles -fno-stack-protector")

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

void print_hex(unsigned long num) {
  char out[30];
  auto ptr_e = &out[29];
  auto ptr = ptr_e;
  *ptr = '0';
  while (num) *(ptr--) = (num % 16 < 10 ? num % 16 + '0' : num % 16 - 10 + 'A'), num /= 16;
  print({ptr + 1, ptr_e + 1});
}

void print(auto* ptr)
  requires(!ivl::meta::same_as_one_of<std::remove_cvref_t<decltype(ptr)>, char*, const char*>)
{
  print("0x");
  print_hex(reinterpret_cast<uintptr_t>(ptr));
}

void __attribute__((noinline)) print(
  ivl::meta::same_as_one_of<
    short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long> auto num
) {
  char memory[30]; // len(str(2**64)) == 20, should be good enough
  unsigned long long abs = num;
  auto end = &memory[0] + 30;
  auto wp = end;
  if (num < 0) abs = -abs;
  do *--wp = abs % 10 + '0', abs /= 10;
  while (abs);
  // asm volatile("" : : "g"(&memory) : "memory");
  if (num < 0) *--wp = '-';
  print(std::string_view(wp, end));
}

void print(auto&&... args)
  requires(sizeof...(args) != 1)
{
  (print(FWD(args)), ...);
}

void println(auto&&... args) { print(FWD(args)..., "\n"); }

void print_error(std::string_view ctx, long error) {
  println(ctx, ": ", -error);
  ivl::linux::raw_syscalls::exit_group(1);
}

namespace ivl {

// TODO: prctl to kill on parent death
struct thread {
  char* stack;
  inline static constexpr size_t stack_size = 4ULL << 12;
  // char stack[stack_size];

  int child_pid;

  thread() : stack(nullptr), child_pid(0) {}

  thread(const thread&) = delete;
  thread& operator=(const thread&) = delete;

  thread(thread&& o) noexcept : stack(o.stack), child_pid(o.child_pid) {
    o.stack = nullptr;
    o.child_pid = 0;
  }

  thread& operator=(thread&& o) noexcept {
    if (this == &o) return *this;
    // TOOD: swap or kill?
    std::swap(stack, o.stack);
    std::swap(child_pid, o.child_pid);
    return *this;
  }

  explicit thread(auto&& callable)
  // requires(!std::same_as<thread, std::remove_cvref_t<decltype(callable)>>)
  {
    using T = std::remove_cvref_t<decltype(callable)>;

    auto ret = ivl::linux::raw_syscalls::mmap(
      0, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS // | MAP_GROWSDOWN
      ,
      -1, 0
    );
    if (ret < 0) print_error("mmap", ret);
    stack = reinterpret_cast<char*>(ret);
    auto stack_end = &stack[0] + stack_size;
    static_assert(sizeof(T) < (1ULL << 12));
    auto moved = new (stack_end - sizeof(T)) T(FWD(callable));
    // asm volatile("" ::"g"(moved) :);

    static_assert(sizeof(T) == 8);

    clone_args args{
      .flags = CLONE_VM | CLONE_FS | CLONE_FILES // | CLONE_THREAD
             | CLONE_SIGHAND | CLONE_IO,
      .pidfd{},
      .child_tid{},
      .parent_tid{},
      .exit_signal = SIGCHLD,
      .stack = reinterpret_cast<uintptr_t>(stack),
      .stack_size = stack_size - sizeof(T),
      .tls{},
      .set_tid{},
      .set_tid_size{},
      .cgroup{},
    };
    volatile auto ret2 = ivl::linux::raw_syscalls::fat_clone3(
      &args, sizeof(args), moved, +[](void* arg) noexcept {
        // register long rsp asm("rsp");
        // (*reinterpret_cast<T*>(rsp + 8))(); // TODO: 8
        (*reinterpret_cast<T*>(arg))();
        ivl::linux::raw_syscalls::exit(0);
        asm volatile("ud2" ::: "memory");
        std::unreachable();
      }
    );
    if (ret2 < 0) print_error("clone3", ret2);
    if (ret2 > 0) {
      child_pid = ret2;
      return;
    }
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

extern "C" __attribute__((naked)) [[noreturn]] void _start() {
  asm volatile("call actual_start\n"
               "ud2\n"
               :
               :
               : "memory");
  __builtin_unreachable();
}

uint64_t __attribute__((noinline)) id(uint64_t x) { return x; }

extern "C" __attribute__((used)) void actual_start() {
  std::atomic<uint64_t> global_sum = 0;
  constexpr uint64_t X = 1ULL << 34;

  auto do_compute = [&](this auto&& self, uint64_t start, uint64_t stride) {
    auto x = __rdtsc();
    uint64_t local_sum = 0;
    for (; start < X; start += stride) local_sum += id(start);
    global_sum += local_sum;
    auto y = __rdtsc();
    println("duration: ", y - x);
  };

  do_compute(0, 1);

  // do_compute(0, 6);
  // do_compute(1, 6);
  // do_compute(2, 6);
  // do_compute(3, 6);
  // do_compute(4, 6);
  // do_compute(5, 6);

  // {
  //   ivl::thread th0{[&] { do_compute(0, 6); }};
  //   ivl::thread th1{[&] { do_compute(1, 6); }};
  //   ivl::thread th2{[&] { do_compute(2, 6); }};
  //   ivl::thread th3{[&] { do_compute(3, 6); }};
  //   ivl::thread th4{[&] { do_compute(4, 6); }};
  //   ivl::thread th5{[&] { do_compute(5, 6); }};
  // }

  println(global_sum.load());
  ivl::linux::raw_syscalls::exit_group(0);
}

// extern "C" [[noreturn]] void _start() {
//   print("Main thread\n");
//   {
//     ivl::thread th([] { print("Thread :)\n"); });
//   }
//   // {
//   //   alignas(4096) char stack[4096];
//   //   clone_args         args{
//   //             .flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_VFORK | CLONE_THREAD | CLONE_SIGHAND | CLONE_IO,
//   //             .pidfd{},
//   //             .child_tid{},
//   //             .parent_tid{},
//   //             .exit_signal = 0,
//   //             .stack       = reinterpret_cast<uintptr_t>(&stack[0]),
//   //             .stack_size  = 4096,
//   //             .tls{},
//   //             .set_tid{},
//   //             .set_tid_size{},
//   //             .cgroup{},
//   //   };
//   //   auto ret = ivl::linux::raw_syscalls::clone3(&args, sizeof(args));
//   //   if (ret < 0) print_error("clone3", ret);
//   //   if (ret == 0) {
//   //     print("Child thread\n");
//   //     ivl::linux::raw_syscalls::exit(0);
//   //   }
//   //   print("Main thread again\n");
//   // }
//   ivl::linux::raw_syscalls::exit_group(0);
//   std::unreachable();
// }
