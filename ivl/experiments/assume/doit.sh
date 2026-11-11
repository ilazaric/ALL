#/usr/bin/env bash

set -euo pipefail

SUFFIX="$1"
if [ "$#" -ge 2 ]
then
    SUFFIX="$1.$2"
fi

uftrace record \
        --no-libcall \
        -P . \
        -d uftrace.data."$SUFFIX" \
/opt/GCC-release/libexec/gcc/x86_64-pc-linux-gnu/17.0.0/cc1plus \
    -quiet -imultiarch x86_64-linux-gnu -D_GNU_SOURCE \
    "$1".cpp \
    -quiet \
    -dumpbase "$SUFFIX".cpp \
    -dumpbase-ext .cpp -march=core-avx2 -O3 -std=c++26 \
    -frandom-seed=foo \
    -o "$SUFFIX".s

uftrace replay -d uftrace.data."$SUFFIX" -f none --no-merge --no-event > etrace."$SUFFIX"

md5sum etrace."$SUFFIX"
wc -l etrace."$SUFFIX"
