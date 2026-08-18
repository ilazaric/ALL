#!/usr/bin/env bash

set -euo pipefail
set -x

A="-Wl,--hash-style=sysv -O3"
B="-shared -fpic"
gcc $A $B -o hash-collision-dso1.{so,c}
gcc $A $B -o hash-collision-dso2.{so,c}
gcc $A $B -o hash-collision-dso3.{so,c}
gcc $A -Wl,-rpath='$ORIGIN' -o hash-collision-main.{exe,c} hash-collision-dso{1,2,3}.so
