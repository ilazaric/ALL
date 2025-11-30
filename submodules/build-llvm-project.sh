#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
OBJ_DIR="$SCRIPT_DIR/objdir/llvm-project"
CMAKE_SRC_DIR="$SCRIPT_DIR/llvm-project/llvm"

mkdir -p "$OBJ_DIR"

cmake \
    -DLLVM_TARGETS_TO_BUILD=X86 \
    -DLLVM_ENABLE_EH=ON \
    -DLLVM_ENABLE_RTTI=ON \
    -DLLVM_ENABLE_PROJECTS="clang;lld" \
    -DLLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS_RELEASE="-O3" \
    -DCMAKE_CXX_FLAGS_RELEASE="-O3" \
    -DCMAKE_CXX_STANDARD=23 \
    -GNinja \
    -S "$CMAKE_SRC_DIR" \
    -B "$OBJ_DIR"

CGROUP_DIR="/sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/compilation_group"
mkdir -p "$CGROUP_DIR"
echo "2000000 100000" > "$CGROUP_DIR/cpu.max"
echo "12G" > "$CGROUP_DIR/memory.max"
echo "0" > "$CGROUP_DIR/memory.swap.max"
echo "0" > "$CGROUP_DIR/memory.zswap.max"
# this would kill everything on OOM, wouldn't really make sense if using `-k 0`
# echo "1" > "$CGROUP_DIR/memory.oom.group"
echo "0" > "$CGROUP_DIR/memory.oom.group"
echo "$$" > "$CGROUP_DIR/cgroup.procs"

exec \
    ninja \
    -C "$OBJ_DIR" \
    "$@"
