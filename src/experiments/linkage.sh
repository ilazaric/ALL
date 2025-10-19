#!/usr/bin/env bash

set -euo pipefail

g++ -std=c++20 -O0 -c linkage.cpp -o linkage.o
g++ -std=c++20 -O0 -c linkage.cpp -o linkage2.o
ld -o linkages.o -r linkage.o linkage2.o
gcc-nm -C linkages.o
