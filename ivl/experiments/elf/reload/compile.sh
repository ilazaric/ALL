#!/usr/bin/env bash

set -euo pipefail
set -x

CC="${CC:-gcc}"

$CC -O3 -fpic -shared foo1.c -o foo1.so
$CC -O3 -fpic -shared foo2.c -o foo2.so
$CC -O3 -Wl,-z,{lazy,undefs} -Wl,--export-dynamic-symbol={foo,puts,dl{open,error,close{,_forced}}} main.c -o main.exe
