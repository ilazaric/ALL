#!/usr/bin/env bsah
set -euo pipefail
# g++ -fcf-protection=none -Wl,-z,noseparate-code -static -nolibc -nostdlib -nostartfiles -fno-stack-protector -O3 -g0 -std=c++23 small2.cpp -c -o small2.o
CXXPRE=-c ivl build small2
mv small2 small2.o
strip --keep-symbol=_start -R .eh_frame -R '*note*' small2.o -o small2.2.o
ld -z noseparate-code small2.2.o -o small2
strip --strip-section-headers small2
ls -lah small2
