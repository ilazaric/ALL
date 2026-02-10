#!/usr/bin/env bash

cd "$(dirname $(realpath "$0"))"

set -euo pipefail
set -x

g++ -Wl,-rpath=/opt/GCC/lib64 -DFORMAT_VERSION='"ilazaric.hpp"' -O3 -std=c++23 bench_format.cpp -o bench_format.ilazaric -lbenchmark
g++ -Wl,-rpath=/opt/GCC/lib64 -DFORMAT_VERSION='"jwakely.hpp"' -O3 -std=c++23 bench_format.cpp -o bench_format.jwakely -lbenchmark

echo "\nilazaric version\n" && taskset -c 1 ./bench_format.ilazaric --benchmark_min_warmup_time=2 --benchmark_repetitions=10
echo "\njwakely  version\n" && taskset -c 1 ./bench_format.jwakely  --benchmark_min_warmup_time=2 --benchmark_repetitions=10
