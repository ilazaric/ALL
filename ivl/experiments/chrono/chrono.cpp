#include <algorithm>
#include <chrono>
#include <cmath>
#include <immintrin.h>
#include <meta>
#include <print>
#include <span>
#include <string_view>
#include <time.h>
#include <vector>
#include <numeric>

// CLOCK_REALTIME
// CLOCK_MONOTONIC
// CLOCK_PROCESS_CPUTIME_ID
// CLOCK_THREAD_CPUTIME_ID
// CLOCK_MONOTONIC_RAW
// CLOCK_REALTIME_COARSE
// CLOCK_MONOTONIC_COARSE
// CLOCK_BOOTTIME
// CLOCK_REALTIME_ALARM
// CLOCK_BOOTTIME_ALARM

[[gnu::noinline]]
void print_header() {
  std::println(
    "clock,"
    "count,"
    "min,5%,10%,25%,50%,75%,90%,95%,"
    "max,"
    "mean,"
    "CV"
  );
}

template<typename Rep>
[[gnu::noinline]]
void print_stats(const std::vector<Rep>& data, std::string_view name) {
  std::vector<double> samples;
  for (std::size_t i = 1; i < data.size(); ++i) {
    if constexpr (std::same_as<Rep, timespec>) {
      samples.push_back(
        (double)(data[i].tv_sec - data[i - 1].tv_sec) * 1'000'000'000.0 +
        (double)(data[i].tv_nsec - data[i - 1].tv_nsec)
      );
    } else {
      using namespace std::literals;
      samples.push_back((data[i] - data[i - 1]) / 1.0ns);
    }
  }
  contract_assert(samples.size() > 10);
  std::ranges::sort(samples);
  auto quantile = [&](double q) {
    // 0 -> 0
    // 100% -> samples.size()-1
    contract_assert(q >= 0 && q <= 1);
    if (q == 1) return samples.back();
    double expanded = q * (samples.size() - 1);
    std::size_t low_index = (std::size_t)expanded;
    std::size_t high_index = low_index + 1;
    double mix = expanded - (double)low_index;
    return samples[low_index] * (1 - mix) + samples[high_index] * mix;
  };
  double sum = 0;
  for (auto sample : samples) sum += sample;
  double mean = sum / (double)samples.size();
  double sum2 = 0;
  for (auto sample : samples) sum2 += sample * sample;
  double pop_var = sum2 / (double)samples.size() - mean * mean;
  double sam_var = pop_var * (double)samples.size() / (double)(samples.size() - 1);
  double sam_std = std::sqrt(sam_var);
  double cv = sam_std / mean;
  // std::println("--- STATS FOR {}", name);
  // std::println("COUNT: {}", samples.size());
  // std::println("MIN:   {}", samples.front());
  // for (auto percent : {5, 10, 25, 50, 75, 90, 95}) std::println("Q {: >2}%: {}", percent, quantile(percent / 100.0));
  // std::println("MAX:   {}", samples.back());
  // std::println("MEAN:  {}", mean);
  // std::println("CV:    {}", cv);
  std::print("{},", name);
  std::print("{},", samples.size());
  std::print("{},", samples.front());
  for (auto percent : {5, 10, 25, 50, 75, 90, 95}) std::print("{},", quantile(percent / 100.0));
  std::print("{},", samples.back());
  std::print("{},", mean);
  std::println("{}", cv);
}

constexpr std::size_t sample_count = 1'000'000;

#define EXPERIMENT(CLOCK)                                                                                              \
  [[gnu::noinline]]                                                                                                    \
  void experiment_##CLOCK() {                                                                                          \
    std::vector<timespec> data(sample_count);                                                                          \
    for (std::size_t i = 0; i < sample_count; ++i) clock_gettime(CLOCK, &data[i]);                                     \
    contract_assert(errno == 0);                                                                                       \
    print_stats(data, #CLOCK);                                                                                         \
  }

EXPERIMENT(CLOCK_REALTIME)
EXPERIMENT(CLOCK_MONOTONIC)
EXPERIMENT(CLOCK_PROCESS_CPUTIME_ID)
EXPERIMENT(CLOCK_THREAD_CPUTIME_ID)
EXPERIMENT(CLOCK_MONOTONIC_RAW)
EXPERIMENT(CLOCK_REALTIME_COARSE)
EXPERIMENT(CLOCK_MONOTONIC_COARSE)
EXPERIMENT(CLOCK_BOOTTIME)
EXPERIMENT(CLOCK_REALTIME_ALARM)
EXPERIMENT(CLOCK_BOOTTIME_ALARM)

#undef EXPERIMENT

template<typename Clock>
[[gnu::noinline]]
void experiment_cxx() {
  std::vector<typename Clock::time_point> data(sample_count);
  for (std::size_t i = 0; i < sample_count; ++i) data[i] = Clock::now();
  print_stats(data, display_string_of(dealias(^^Clock)));
}

consteval std::vector<std::meta::info> find_all_clocks() {
  std::vector<std::meta::info> ret;
  std::vector<std::meta::info> candidates = members_of(^^std::chrono, std::meta::access_context::unchecked());
  for (std::size_t i = 0; i < candidates.size(); ++i) {
    auto member = candidates[i];
    if (is_namespace(member)) {
      candidates.insert_range(candidates.end(), members_of(member, std::meta::access_context::unchecked()));
      continue;
    }
    if (!is_type(member)) continue;
    if (!extract<bool>(substitute(
          ^^std::chrono::is_clock_v, {
                                       member
                                     }
        )))
      continue;
    __builtin_constexpr_diag(32, "clock", std::format("found clock: {}", display_string_of(member)));
    ret.push_back(member);
  }
  return ret;
}

struct rdtsc_clock {
  using rep = decltype(_rdtsc());
  using period = std::ratio<1, 2'500'000'000>;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<rdtsc_clock>;
  inline static constexpr bool is_steady = true;
  static time_point now() noexcept { return time_point(duration(_rdtsc())); }
};

struct cpuid_ret {
  unsigned int eax;
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;
};

// stolen from linux:

static inline void native_cpuid(unsigned int* eax, unsigned int* ebx, unsigned int* ecx, unsigned int* edx) {
  /* ecx is often an input as well as an output. */
  asm volatile("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "0"(*eax), "2"(*ecx) : "memory");
}

/*
 * Generic CPUID function
 * clear %ecx since some cpus (Cyrix MII) do not set or clear %ecx
 * resulting in stale register contents being returned.
 */
static inline void
cpuid_count(unsigned int op, int count, unsigned int* eax, unsigned int* ebx, unsigned int* ecx, unsigned int* edx) {
  *eax = op;
  *ecx = count;
  native_cpuid(eax, ebx, ecx, edx);
}

cpuid_ret cpuid(unsigned int op, int count = 0) {
  cpuid_ret ret;
  cpuid_count(op, count, &ret.eax, &ret.ebx, &ret.ecx, &ret.edx);
  return ret;
}

std::uint64_t compute_tsc() {
  auto max_leaf = cpuid(0x0).eax;
  auto max_extended_leaf = cpuid(0x80000000).eax;
  if (max_extended_leaf == max_leaf) {
    std::println(stderr, "WARN: cpuid(0x80000000) not supported, cannot verify TSC invariant");
  } else if (max_extended_leaf < 0x80000007) {
    std::println(stderr, "WARN: cpuid(0x80000007) not supported, cannot verify TSC invariant");
  } else {
    auto tsc_invariant = (cpuid(0x80000007).edx >> 8) & 1;
    if (tsc_invariant == 1) {
      std::println(stderr, "INFO: TSC invariant validated");
    } else {
      std::println(stderr, "WARN: TSC invariant not validated");
    }
  }
  if (max_leaf < 0x15) {
    std::println(stderr, "ERROR: cpuid supports inputs up to {:#x}, we need 0x15 at least", max_leaf);
    std::exit(1);
  }
  auto [denominator, numerator, nominal_art_frequency, _] = cpuid(0x15);
  if (nominal_art_frequency != 0) {
    auto tsc_freq = (std::uint64_t)nominal_art_frequency * numerator / denominator;
    std::println(stderr, "INFO: cpuid(0x15) reports clock frequency, high quality path");
    std::println(stderr, "INFO: computed TSC frequency: {}", tsc_freq);
    return tsc_freq;
  }
  std::println(stderr, "WARN: cpuid(0x15) does not report clock frequency, falling back to cpuid(0x16)");
  if (max_leaf < 0x16) {
    std::println(stderr, "ERROR: cpuid supports inputs up to {:#x}, we need 0x16", max_leaf);
    std::exit(1);
  }
  auto processor_base_frequency = (std::uint64_t)(cpuid(0x16).eax & ((1u << 16) - 1));
  processor_base_frequency *= 1'000'000;
  std::println(stderr, "WARN: using base frequency as TSC frequency: {}", processor_base_frequency);
  return processor_base_frequency;
}

const std::uint64_t tsc_freq = compute_tsc();

struct runtime_ratio_tag {};

template<typename Period>
std::intmax_t generic_num() {
  if constexpr (std::derived_from<Period, runtime_ratio_tag>) {
    return Period::num();
  } else {
    return Period::num;
  }
}

template<typename Period>
std::intmax_t generic_den() {
  if constexpr (std::derived_from<Period, runtime_ratio_tag>) {
    return Period::den();
  } else {
    return Period::den;
  }
}

std::pair<std::intmax_t, std::intmax_t> reduce_fraction(std::intmax_t num1, std::intmax_t den1, std::intmax_t num2, std::intmax_t den2) {
  auto reduce = [](std::intmax_t& num, std::intmax_t& den) {
    auto g = std::gcd(num, den);
    num /= g;
    den /= g;
  };
  template for (auto& num : {num1, num2}) {
    template for (auto& den : {den1, den2}) {
      reduce(num, den);
    }
  }
  std::intmax_t num;
  std::intmax_t den;
  bool overflow_num = __builtin_mul_overflow(num1, num2, &num);
  bool overflow_den = __builtin_mul_overflow(den1, den2, &den);
  contract_assert(!overflow_num && !overflow_den);
  return {num, den};
}

template<typename Rep1, typename Period1,
         typename Rep2, typename Period2>
requires (std::derived_from<Period1, runtime_ratio_tag> || std::derived_from<Period2, runtime_ratio_tag>)
struct std::common_type<std::chrono::duration<Rep1, Period1>, std::chrono::duration<Rep2, Period2>> {
  struct period : runtime_ratio_tag {
    using type = period;
    inline static std::pair<std::intmax_t, std::intmax_t> num_den() {
      static std::pair<std::intmax_t, std::intmax_t> cached = reduce_fraction(generic_num<Period1>(), generic_den<Period1>(), generic_num<Period2>(), generic_den<Period2>());
      return cached;
    }
    inline static std::intmax_t num() { return num_den().first; }
    inline static std::intmax_t den() { return num_den().second; }
  };
  using type = std::chrono::duration<std::common_type<Rep1, Rep2>, period>;
};

template<typename Rep, std::derived_from<runtime_ratio_tag> Period>
struct std::chrono::duration<Rep, Period> {
  using rep = Rep;
  using period = Period;

  rep r;

  duration() = default;
  duration(const duration&) = default;
  ~duration() = default;

  template<typename Dur>
  requires std::__is_duration<Dur>::value
  duration(const Dur& d) : r(std::duration_cast<duration>(d).count()) {}
};

struct rdtsc_clock2 {
  using rep = decltype(_rdtsc());
  struct period : runtime_ratio_tag {
    using type = period;
    inline static std::intmax_t num() { return 1; }
    inline static std::intmax_t den() { return tsc_freq; }
  };
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<rdtsc_clock2>;
  inline static constexpr bool is_steady = true;
  static time_point now() noexcept { return time_point(duration(_rdtsc())); }
};

#include <ivl/logger>

constexpr uint64_t MASK = (1ULL << 16) - 1;
constexpr uint64_t EAX = 1ULL << 0;
constexpr uint64_t EBX = 1ULL << 16;
constexpr uint64_t ECX = 1ULL << 32;
constexpr uint64_t EDX = 1ULL << 48;

constexpr uint64_t DEC = 0;
constexpr uint64_t HEX = 1;
constexpr uint64_t STR = 2;

void print_reg(unsigned int reg, uint64_t ctl) {
  if (ctl == DEC) std::print("{}", reg);
  else if (ctl == HEX) std::print("{:#x}", reg);
  else if (ctl == STR) std::print("{:?}", std::string_view((const char*)&reg, (const char*)(&reg + 1)));
  else {
    std::println(stderr, "unrecognized ctl: {}", ctl);
    std::terminate();
  }
}

void cpuid_log(unsigned int op, int count = 0, uint64_t ctl = 0) {
  auto ret = cpuid(op, count);
  std::print("op={:#x}", op);
  if (count) std::print(" count={:#x}", count);
  std::print(" eax=");
  print_reg(ret.eax, (ctl / EAX) & MASK);
  std::print(" ebx=");
  print_reg(ret.ebx, (ctl / EBX) & MASK);
  std::print(" ecx=");
  print_reg(ret.ecx, (ctl / ECX) & MASK);
  std::print(" edx=");
  print_reg(ret.edx, (ctl / EDX) & MASK);
  std::println();
}

auto carve(unsigned int num, auto... widths) -> std::array<unsigned int, sizeof...(widths)> {
  contract_assert((0 + ... + widths) == 32);
  auto lambda = [&](unsigned int width) {
    auto ret = num ^ (num >> width << width);
    num >>= width;
    return ret;
  };
  return {lambda(widths)...};
}

int main() {
  if (0) {
    cpuid_log(0x15);
    cpuid_log(0, 0, (EBX | ECX | EDX) * STR);
    // cpuid_log(1); // version multiplexed
    // cpuid_log(2, 0, (EAX|EBX|ECX|EDX)*HEX);

    std::println();
    for (int count = 0;; ++count) {
      auto regs = cpuid(4, count);
      if (regs.eax == 0) break;

      auto
        [cache_type, cache_level, self_initializing_cache, fully_assoc, _, max_lp_addressable_ids,
         max_cores_addressable_ids_pkg] = carve(regs.eax, 5, 3, 1, 1, 4, 12, 6);
      auto [line_size, phys_line_partitions, num_ways] = carve(regs.ebx, 12, 10, 10);
      auto [num_sets] = carve(regs.ecx, 32);
      auto [not_lwr_cache_flush, inclusive_cache, complex_cache_indexing, _] = carve(regs.edx, 1, 1, 1, 29);

      if (cache_type == 0) break;

      LOG(count);
      LOG(cache_type);
      LOG(cache_level);
      LOG(self_initializing_cache);
      LOG(fully_assoc);
      LOG(max_lp_addressable_ids);
      LOG(max_cores_addressable_ids_pkg);
      LOG(line_size);
      LOG(phys_line_partitions);
      LOG(num_ways);
      LOG(num_sets);
      LOG(not_lwr_cache_flush);
      LOG(inclusive_cache);
      LOG(complex_cache_indexing);
      std::println();
    }

    return 0;
  }

  // {
  //   unsigned int eax_denominator, ebx_numerator, ecx_hz, edx;
  //   eax_denominator = ebx_numerator = ecx_hz = edx = 0;
  //   /* CPUID 15H TSC/Crystal ratio, plus optionally Crystal Hz */
  //   cpuid(0x15, 0, &eax_denominator, &ebx_numerator, &ecx_hz, &edx);
  //   LOG(eax_denominator, ebx_numerator, ecx_hz, edx);
  // }

  print_header();

#define RUN(CLOCK) experiment_##CLOCK()
  RUN(CLOCK_REALTIME);
  RUN(CLOCK_MONOTONIC);
  RUN(CLOCK_PROCESS_CPUTIME_ID);
  RUN(CLOCK_THREAD_CPUTIME_ID);
  RUN(CLOCK_MONOTONIC_RAW);
  RUN(CLOCK_REALTIME_COARSE);
  RUN(CLOCK_MONOTONIC_COARSE);
  RUN(CLOCK_BOOTTIME);
  RUN(CLOCK_REALTIME_ALARM);
  RUN(CLOCK_BOOTTIME_ALARM);
#undef RUN

  template for (constexpr auto clock : define_static_array(find_all_clocks())) { experiment_cxx<typename[:clock:]>(); }

  experiment_cxx<rdtsc_clock>();
  experiment_cxx<rdtsc_clock2>();
}
