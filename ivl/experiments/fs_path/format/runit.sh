#!/usr/bin/env bash

cd "$(dirname $(realpath "$0"))"

set -euo pipefail
set -x

versions=(ilazaric jwakely)

for version in ${versions[@]}
do
    g++ -flto -march=native -Wl,-rpath=/opt/GCC/lib64 -O3 -std=c++23 \
        -Wl,--emit-relocs -fno-reorder-blocks-and-partition \
        -I ../../../../submodules/google-benchmark/include/ \
        -DFORMAT_VERSION="\"$version.hpp\"" \
        bench_format.cpp ../../../../submodules/google-benchmark/src/*.cc \
        -o bench_format.$version
done

for version in ${versions[@]}
do
    sudo perf record -e cycles:u -j any,u -o bench_format.$version.data -- ./bench_format.$version
    sudo chown ilazaric bench_format.$version.data
    chmod +rw bench_format.$version.data
    perf2bolt -p bench_format.$version.data -o bench_format.$version.fdata bench_format.$version
    llvm-bolt bench_format.$version -o bench_format.$version.bolt -data=bench_format.$version.fdata \
              -reorder-blocks=ext-tsp -reorder-functions=cdsort -split-functions -split-all-cold -split-eh -dyno-stats
done

for version in ${versions[@]}
do
    echo
    echo "$version version"
    echo
    taskset -c 1 ./bench_format.$version.bolt --benchmark_min_warmup_time=2 --benchmark_repetitions=10
done > bench.txt
