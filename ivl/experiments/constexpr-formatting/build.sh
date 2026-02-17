#!/usr/bin/env bash

set -euo pipefail
set -x

rsync -avz \
      /home/ilazaric/repos/ALL/submodules/gcc/libstdc++-v3/include/bits/ \
      /opt/GCC/include/c++/16.0.1/bits/

rsync -avz \
      /home/ilazaric/repos/ALL/submodules/gcc/libstdc++-v3/include/ext/ \
      /opt/GCC/include/c++/16.0.1/ext/

function copy_std() {
    cp /home/ilazaric/repos/ALL/submodules/gcc/libstdc++-v3/include/std/$1 /opt/GCC/include/c++/16.0.1/$1
}

copy_std format
copy_std map
copy_std set

g++ -Wl,-rpath=/opt/GCC/lib64 -std=c++26 -freflection \
    -fsanitize=address,undefined \
    "$1.cpp" -o "$1"
#    -o constexpr_to_string{,.cpp}
