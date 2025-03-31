#!/usr/bin/env bash

set -euo pipefail

echo '#pragma once'
echo

echo '#include <map>'
echo '#include <string>'
echo

echo "std::map<std::string, int> syscall2number{"
cat /usr/include/x86_64-linux-gnu/asm/unistd_64.h | grep define | grep NR | cut -d '_' -f 4- | tr ' ' '/' | parallel -j1 'echo "  { \"{//}\", {/} },"'
echo '};'
echo

echo "std::map<int, std::string> number2syscall{"
cat /usr/include/x86_64-linux-gnu/asm/unistd_64.h | grep define | grep NR | cut -d '_' -f 4- | tr ' ' '/' | parallel -j1 'echo "  { {/}, \"{//}\" },"'
echo '};'
echo

