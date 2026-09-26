#!/usr/bin/env bash

set -euo pipefail

RUNCOUNT=5

function compile_check() {
    g++ ${CXXOPTS-} F2v4.cpp -DCHOICE="$1" -fconstexpr-ops-limit=1000000000000 -std=c++23 -o "F2v4.$1.exe" \
        && echo "compiles: PASS" || { echo "compiles: FAIL"; return 1; }
}

function runtime_check() {
    bash -c "./F2v4.$1.exe && exit 0 || exit 1" &> /dev/null \
        && echo "runtime out-of-bounds test: FAIL"  \
        || echo "runtime out-of-bounds test: PASS"
}

function perf_sanity_check() {
    [ "$(cat "F2v4.$1.out" | grep "EXIT CODE: 0" | wc -l)" -eq "$RUNCOUNT" ] || \
        { echo "perf sanity check: FAIL"; return 1; }
    [ "$(cat "F2v4.$1.out" | grep "IVL" | uniq | wc -l)" -eq "1" ] || \
        { echo "perf sanity check: FAIL"; return 1; }
    echo "perf sanity check: PASS"
}

for CHOICE in BASELINE STDLIB_DBG IVL_{CONSTEVAL,ALWAYS{,_SIZE,_COND},BUILTIN,ALWAYS_V2,ALWAYS_EXPECT}
do
    echo
    echo "evaluating CHOICE=$CHOICE ..."
    compile_check "$CHOICE" || continue
    runtime_check "$CHOICE"
    ivl script perf-run --copy F2v4.cpp --repeat "$RUNCOUNT" -- \
        IVL_GCC_DUMP_CONSTEXPR_OPS_COUNT= \
        g++ ${CXXOPTS-} F2v4.cpp -DCHOICE="$CHOICE" -fsyntax-only -fconstexpr-ops-limit=1000000000000 -std=c++23 \
        > "F2v4.$CHOICE.out"
    perf_sanity_check "$CHOICE" || continue
    OPS="$(cat "F2v4.$CHOICE.out" | grep IVL | head -1 | rev | cut -d ' ' -f 1 | rev)"
    DUR="$(cat "F2v4.$CHOICE.out" | grep '^user'$'\t' \
        | cut -d $'\t' -f 2 | cut -c 3- | rev | cut -c 2- | rev \
        | awk '{s+=$1}END{print s/NR}')"
    echo "${DUR}s, ${OPS}ops"
done
echo
