#!/usr/bin/env bash

set -euo pipefail

uftrace record --no-libcall -P . /opt/GCC-release/libexec/gcc/x86_64-pc-linux-gnu/17.0.0/cc1plus -quiet -imultiarch x86_64-linux-gnu -D_GNU_SOURCE ubsan-bug.cpp -quiet -dumpbase ubsan-bug.cpp -dumpbase-ext .cpp -mtune=generic -march=x86-64 -std=c++26 -o /tmp/ccywqEBz.s
