#include "evil_lexically_normal.hpp"
#include "sane_lexically_normal.hpp"
#include <benchmark/benchmark.h>

static void PathConstruction(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    std::filesystem::path p{"/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path/"};
    // Make sure the variable is not optimized away by compiler
    benchmark::DoNotOptimize(p);
  }
}
BENCHMARK(PathConstruction);

static void PathCopy(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    std::filesystem::path p{"/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path/"};
    // Make sure the variable is not optimized away by compiler
    benchmark::DoNotOptimize(auto(p));
  }
}
BENCHMARK(PathCopy);

static void PathNormalization(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    std::filesystem::path p{"/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path/"};
    // Make sure the variable is not optimized away by compiler
    benchmark::DoNotOptimize(p.lexically_normal());
  }
}
BENCHMARK(PathNormalization);

static void EvilPathNormalization(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    std::filesystem::path p{"/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path/"};
    // Make sure the variable is not optimized away by compiler
    benchmark::DoNotOptimize(impl::lexically_normal(p));
  }
}
BENCHMARK(EvilPathNormalization);

static void SanePathNormalization(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    std::filesystem::path p{"/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path/"};
    // Make sure the variable is not optimized away by compiler
    benchmark::DoNotOptimize(impl_sane::lexically_normal(p));
  }
}
BENCHMARK(SanePathNormalization);
