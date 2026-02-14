#!/usr/bin/env bash

set -euo pipefail
set -x

function compile() {
    g++ -Wl,-rpath=/opt/GCC/lib64 -std=c++26 -O3 -freflection \
        -xc++ -include "$1.cpp" -include lib.hpp /dev/null \
        -o "$1"
}

function compile_obj() {
    g++ -Wl,-rpath=/opt/GCC/lib64 -std=c++26 -O3 -freflection \
        -xc++ -include "$1.cpp" -include lib.hpp /dev/null \
        -o "$1.o" -c
}

function link() {
    g++ -Wl,-rpath=/opt/GCC/lib64 -std=c++26 -O3 -freflection "$@"
}

function test_ec() {
    "./$1" || ec="$?"
    [ "$ec" -eq "$2" ]
}

function compile_and_test() {
    compile "$1"
    test_ec "$1" "$2"
}

compile_and_test main 1
compile_and_test main2 2
compile_and_test main3 3
compile_and_test  ivl_main 4

compile_obj tu_fn
compile_obj tu_main
compile_obj tu_ivl_main

link tu_fn.o tu_main.o -o tu_main
link tu_fn.o tu_ivl_main.o -o tu_ivl_main

test_ec tu_main 5
test_ec tu_ivl_main 5
