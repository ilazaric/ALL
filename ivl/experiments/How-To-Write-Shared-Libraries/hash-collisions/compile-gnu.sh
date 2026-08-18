#!/usr/bin/env bash

set -euo pipefail
set -x

A="-Wl,--hash-style=gnu -O3"
B="-shared -fpic"
gcc $A $B -o gnu-hash-collision-dso1.{so,c}
gcc $A $B -o gnu-hash-collision-dso2.{so,c}
gcc $A $B -o gnu-hash-collision-dso3.{so,c}
gcc $A -Wl,-rpath='$ORIGIN' -o gnu-hash-collision-main.{exe,c} gnu-hash-collision-dso{1,2,3}.so
