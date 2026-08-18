#!/usr/bin/env bash

set -euo pipefail
set -x

for kind in sysv gnu
do
    A="-Wl,--hash-style=$kind -O3"
    B="-shared -fpic"
    gcc $A $B -o both-$kind-hash-collision-dso1.so both-hash-collision-dso1.c
    gcc $A $B -o both-$kind-hash-collision-dso2.so both-hash-collision-dso2.c
    gcc $A $B -o both-$kind-hash-collision-dso3.so both-hash-collision-dso3.c
    gcc $A -Wl,-rpath='$ORIGIN' -o both-$kind-hash-collision-main.exe both-hash-collision-main.c both-$kind-hash-collision-dso{1,2,3}.so
done
