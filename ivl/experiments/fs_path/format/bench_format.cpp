#include <benchmark/benchmark.h>
#include FORMAT_VERSION

// just to get syntax_only test to pass, runit.sh is actual compilation
// IVL add_compiler_flags("-DFORMAT_VERSION=\"ilazaric\"")

// IVL add_compiler_flags_tail("-lbenchmark")

static void PathFormat(benchmark::State& state) {
  std::filesystem::path p{"/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"};
  for (auto _ : state) {
    benchmark::DoNotOptimize(std::format("{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}", *(ivl::path*)&p));
  }
}
BENCHMARK(PathFormat);

static void PathWideFormat(benchmark::State& state) {
  std::filesystem::path p{"/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"};
  for (auto _ : state) {
    benchmark::DoNotOptimize(std::format(L"{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}", *(ivl::path*)&p));
  }
}
BENCHMARK(PathWideFormat);
