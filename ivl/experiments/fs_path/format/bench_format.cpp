#include <benchmark/benchmark.h>
#include FORMAT_VERSION

// IVL add_compiler_flags_tail("-lbenchmark")

static void PathFormat(benchmark::State& state) {
  std::filesystem::path p{"/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"
                          "/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path"};
  for (auto _ : state) {
    benchmark::DoNotOptimize(std::format("{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}", p));
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
    benchmark::DoNotOptimize(std::format(L"{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}", p));
  }
}
BENCHMARK(PathWideFormat);
