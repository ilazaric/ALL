#!/usr/bin/env bash

set -euo pipefail

CXX=/home/ilazaric/GCC-16/bin/g++ ivl build blake3_example

for ((i=0; i<10000; ++i))
do
    diff <( head -c $i /dev/zero | b3sum ) \
         <( head -c $i /dev/zero | ./blake3_example )
done
echo "PASSED NULL TEST"

for ((i=0; i<5000; ++i))
do
    diff <( head -c $i blake3.hpp | b3sum ) \
         <( head -c $i blake3.hpp | ./blake3_example )
done
echo "PASSED HEADER TEST"

# function f() {
#     cat blake3.hpp
#     head -c 1000000 /dev/zero
# }
# f | ./blake3_example
# f | b3sum
