#!/usr/bin/env bash

set -euo pipefail
set -x

g++ -Wl,-rpath=/opt/GCC/lib64 -std=c++26 -O3 -freflection \
    -xc++ -include "$1.cpp" -include lib.hpp /dev/null \
    -o "$1"
