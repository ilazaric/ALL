#!/usr/bin/env bash
set -euo pipefail
IVL_WARN= IVL_GCC_DUMP_CONSTEXPR_OPS_COUNT= /home/ilazaric/repos/ALL/submodules/objdir/gcc-release/gcc/xg++ -B/home/ilazaric/repos/ALL/submodules/objdir/gcc-release/gcc -std=c++26 -fsyntax-only F2v5.cpp -O0 &> O0.out
IVL_WARN= IVL_GCC_DUMP_CONSTEXPR_OPS_COUNT= /home/ilazaric/repos/ALL/submodules/objdir/gcc-release/gcc/xg++ -B/home/ilazaric/repos/ALL/submodules/objdir/gcc-release/gcc -std=c++26 -fsyntax-only F2v5.cpp -Og &> Og.out
