#!/usr/bin/env bash

set -euo pipefail

#  -fdump-ipa-cp-details -fdump-tree-optimized

for f in inherit.0_1_0_1.cpp inherit.1_1_0_1.cpp inherit.0_1_1_1.cpp
do
    g++ -std=c++26 -O3 -march=core-avx2 -frandom-seed=foo \
        -fdump-ipa-all-details -fdump-tree-all-details \
        -S "$f"
done
