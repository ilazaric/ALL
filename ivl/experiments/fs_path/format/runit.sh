#!/usr/bin/env bash

cd "$(dirname $(realpath "$0"))"

set -euo pipefail
set -x

versions=(ilazaric jwakely)

for version in ${versions[@]}
do
    g++ -Wl,-rpath=/opt/GCC/lib64 -DFORMAT_VERSION="\"$version.hpp\"" -O3 -std=c++23 bench_format.cpp -o bench_format.$version -lbenchmark
done

for version in ${versions[@]}
do
    echo
    echo "$version version"
    echo
    taskset -c 1 ./bench_format.$version --benchmark_min_warmup_time=2 --benchmark_repetitions=10
done
