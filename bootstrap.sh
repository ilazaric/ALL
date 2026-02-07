#!/usr/bin/env bash

set -euo pipefail
set -x

# if ! [ -d /opt/GCC ]
# then
#     git submodule update --init --depth=1 -- submodules/gcc
#     submodules/build-gcc.sh --install
# fi

# export PATH="/opt/GCC/bin:$PATH"
# export LC_ALL="C"

ROOT="$(git rev-parse --show-toplevel)"
DIR="$ROOT/build/bootstrap_dir"

mkdir -p "$DIR"

if ! [ -f "$DIR/generate_build_sources" ]
then
    g++ -Wl,-rpath=/opt/GCC/lib64 \
        "$ROOT/ivl/build_system/generate_build_sources.cpp" \
        -O3 -std=c++26 -o "$DIR/generate_build_sources"
fi

if ! [ -f "$DIR/builder" ]
then
    "$DIR/generate_build_sources"
    g++ -Wl,-rpath=/opt/GCC/lib64 \
        @"$ROOT/build/include_dirs/args.rsp" -freflection \
        "$ROOT/ivl/build_system/builder.cpp" \
        -O3 -std=c++26 -o "$DIR/builder" -lstdc++exp
fi

exec "$DIR/builder" "$@"
