#!/usr/bin/env bash

set -euo pipefail

# -x comms.gdb
# uftrace record -v -P mark_exp_read \

gdb -x comms.gdb --args \
        /usr/libexec/gcc/x86_64-linux-gnu/13/cc1plus -quiet -imultiarch \
        x86_64-linux-gnu -D_GNU_SOURCE ubsan-bug.cpp -mtune=generic -march=x86-64 \
        -fasynchronous-unwind-tables -fstack-protector-strong -Wformat -Wformat-security \
        -fstack-clash-protection -fcf-protection -dumpbase ubsan-bug.cpp -dumpbase-ext .cpp
