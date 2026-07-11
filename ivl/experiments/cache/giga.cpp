#include <ivl/linux/throwing_syscalls>
#include <ivl/logger>
#include <linux/mman.h>
#include <cstring>
#include <immintrin.h>

// IVL add_compiler_flags("-static")

struct cache_stats {
  std::size_t core_accessible_size;
  std::size_t associativity;
  std::size_t number_of_sets;
};

constexpr cache_stats l1i{
  .core_accessible_size = (128 << 10) / 4,
  .associativity = 8,
  .number_of_sets = 64,
};

constexpr cache_stats l1d{
  .core_accessible_size = (128 << 10) / 4,
  .associativity = 8,
  .number_of_sets = 64,
};

constexpr cache_stats l2{
  .core_accessible_size = (1 << 20) / 4,
  .associativity = 4,
  .number_of_sets = 1024,
};

constexpr cache_stats l3{
  .core_accessible_size = (6 << 20) / 1,
  .associativity = 12,
  .number_of_sets = 8192,
};

constexpr bool checks_out(const cache_stats& stats) {
  return stats.associativity * stats.number_of_sets * 64 == stats.core_accessible_size;
}

static_assert(checks_out(l1i));
static_assert(checks_out(l1d));
static_assert(checks_out(l2));
static_assert(checks_out(l3));

constexpr std::size_t page_size = 1 << 30;

struct alignas(64) cache_line {
  cache_line* next;
  std::size_t unused[7];
};

static_assert(sizeof(cache_line) == 64);
static_assert(alignof(cache_line) == 64);

template<class Tp>
[[gnu::always_inline]] inline void DoNotOptimize(Tp const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

// dont use, go through multi_run
[[gnu::always_inline]]
inline std::size_t single_run(cache_line* start) {
  std::size_t cycle_start = _rdtsc();
  auto current = start;
  do {
    cycle_start += current->unused[0]; // zero
    current = current->next;
  } while (current != start);
  // DoNotOptimize(current);
  std::size_t cycle_end = _rdtsc();
  return cycle_end - cycle_start - 25 /* noop */;
}

// TODO: take a look at the asm
[[gnu::noinline]]
std::size_t multi_run(cache_line* start, std::size_t n) {
  DoNotOptimize(single_run(start)); // to warm up the memory, so every run is uniform
  std::size_t ret = 0;
  while (n--) ret += single_run(start);
  return ret;
}

// dont use, go through multi_run
[[gnu::always_inline]]
inline std::size_t single_run2(cache_line* start) {
  std::size_t cycle_start = 0;
  auto current = start;
  do {
    cycle_start += current->unused[0]; // zero
    current = current->next;
  } while (current != start);
  return cycle_start;
}

// TODO: take a look at the asm
[[gnu::noinline]]
std::size_t multi_run2(cache_line* start, std::size_t n) {
  DoNotOptimize(single_run(start)); // to warm up the memory, so every run is uniform
  std::size_t cycle_start = _rdtsc();
  while (n--) cycle_start += single_run2(start);
  std::size_t cycle_end = _rdtsc();
  return cycle_end - cycle_start - 25;
}

// TODO: take a look at the asm
[[gnu::noinline]]
std::size_t multi_run3(cache_line* start, std::size_t n) {
  auto current = start;
  asm volatile(".Lwarmup_start_%=:\n\t"
               "mov (%0), %0\n\t"
               "cmp %0, %1\n\t"
               "jne .Lwarmup_start_%=\n\t"
               : "+r"(current), "+r"(start) /* outputs */
               :                            /* inputs */
               : /* clobbers */);
  std::size_t ret = _rdtsc();
  while (n--) {
    asm volatile(".Lbody_start_%=:\n\t"
                 "mov (%0), %0\n\t"
                 "cmp %0, %1\n\t"
                 "jne .Lbody_start_%=\n\t"
                 : "+r"(current), "+r"(start), "+r"(ret) /* outputs */
                 :                                       /* inputs */
                 : /* clobbers */);
  }
  std::size_t e = _rdtsc();
  return e - ret;
}

// TODO: take a look at the asm
// TODO: i think this is a bit incorrect, constraints should say it reads from memory
[[gnu::noinline]]
std::size_t multi_run4(cache_line* start, std::size_t n) {
  auto current = start;
  asm volatile(".Lwarmup_start_%=:\n\t"
               "test %0, %0\n\t"
               "jz .Lwarmup_end_%=\n\t"
               "mov (%0), %0\n\t"
               "jmp .Lwarmup_start_%=\n\t"
               ".Lwarmup_end_%=:\n\t"
               : "+r"(current), "+r"(start) /* outputs */
               :                            /* inputs */
               : /* clobbers */);
  std::size_t ret = _rdtsc();
  while (n--) {
    current = start;
    asm volatile(".Lbody_start_%=:\n\t"
                 "test %0, %0\n\t"
                 "jz .Lbody_end_%=\n\t"
                 "mov (%0), %0\n\t"
                 "jmp .Lbody_start_%=\n\t"
                 ".Lbody_end_%=:\n\t"
                 : "+r"(current), "+r"(start), "+r"(ret) /* outputs */
                 :                                       /* inputs */
                 : /* clobbers */);
  }
  std::size_t e = _rdtsc();
  return e - ret;
}

[[gnu::noinline]]
std::size_t noop() {
  std::size_t a = __rdtsc();
  std::size_t b = __rdtsc();
  return b - a;
}

int ivl_main() {
  LOG(noop());
  namespace sys = ivl::linux::throwing_syscalls;
  auto fd = sys::open("/dev/hugepages/giga", O_RDWR, 0);
  auto addr = reinterpret_cast<void*>(
    sys::mmap(0, 1 << 30, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_HUGE_1GB, fd, 0)
  );
  LOG(addr);
  contract_assert(reinterpret_cast<uintptr_t>(addr) % 64 == 0);
  memset(addr, 0, page_size);

  std::span<cache_line> lines((cache_line*)addr, page_size / sizeof(cache_line));

  lines[0].next = &lines[0];
  // auto last = multi_run(&lines[0], 1000);
  // auto last2 = multi_run2(&lines[0], 1000);
  auto noop_time = noop();
  auto last3 = multi_run3(&lines[0], 1000) - noop_time;
  // auto last4 = multi_run4(nullptr, 1000);
  LOG(noop_time);
  LOG(last3);
  for (int i = 1; i <= 20; ++i) {
    // for (int x = 0; x < 100; ++x) LOG((void*)&lines[x], (void*)lines[x].next);
    lines[(i - 1) * l1d.number_of_sets].next = &lines[i * l1d.number_of_sets];
    lines[i * l1d.number_of_sets].next = &lines[0];
    // auto cyc = multi_run(&lines[0], 1000);
    // auto cyc2 = multi_run2(&lines[0], 1000);
    auto cyc3 = multi_run3(&lines[0], 1000) - noop_time;
    // auto cyc4 = multi_run4(&lines[0], 1000);
    // LOG(i, cyc - last);
    // LOG(i, cyc2 - last2);
    LOG(i, i + 1, cyc3 - last3, cyc3 / (i + 1));
    // last = cyc;
    // last2 = cyc2;
    last3 = cyc3;
    // last4 = cyc4;
  }
  return 0;
}
