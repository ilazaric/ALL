#!/usr/bin/env bash

set -euo pipefail
set -x

DATADIR="$(dirname "$(realpath "$0")")"
BENCHMARK="$DATADIR/../binary_tree_traversal_benchmark"
HOST="debian-perf-1"
OUTPUT="$DATADIR/output"
CSV="$DATADIR/data.csv"
RUNNER="$DATADIR/run.sh"
DELAYED_EXEC="$DATADIR/../delayed_exec"

[ -f "$BENCHMARK" ]
[ -f "$CSV" ]
[ -f "$RUNNER" ]
[ -f "$DELAYED_EXEC" ]

TIMESTAMP="$(date +%s%N)"

rsync -a "$BENCHMARK" "$HOST:/tmp/benchmark"
rsync -a "$RUNNER" "$HOST:/tmp/runner"
rsync -a "$DELAYED_EXEC" "$HOST:/tmp/delayed_exec"
ssh "$HOST" -- "/tmp/runner" > "$OUTPUT" && EC="$?" || EC="$?"

[ "$(cat "$OUTPUT" | wc -l)" -eq 11 ]

function output_line() {
    LINE="$(head "-$1" $OUTPUT | tail -1)"
    [ "$(echo "$LINE" | cut -d ' ' -f 1)" = "$2:" ]
    [ "$(echo "$LINE" | cut -d ' ' -f 3- )" = "" ]
    echo "$LINE" | cut -d ' ' -f 2
}

CPUFREQ_BEFORE="$(output_line 1 "cpufreq_before")"
DURATION="$(output_line 2 "duration")"
EVIDENCE="$(output_line 3 "evidence")"
CPUFREQ_AFTER="$(output_line 4 "cpufreq_after")"
PERF_CYCLES="$(output_line 5 "perf:cycles")"
PERF_CTX="$(output_line 6 "perf:context-switches")"
PERF_MIG="$(output_line 7 "perf:cpu-migrations")"
PERF_FAULT="$(output_line 8 "perf:page-faults")"
PERF_L3_MISS="$(output_line 9 "perf:cache-misses")"
PERF_L2_MISS="$(output_line 10 "perf:l2_rqsts.all_demand_miss")"
PERF_L1_MISS="$(output_line 11 "perf:L1-dcache-misses")"

PERF="$PERF_CYCLES,$PERF_CTX,$PERF_MIG,$PERF_FAULT,$PERF_L3_MISS,$PERF_L2_MISS,$PERF_L1_MISS"

echo "$DURATION,$EVIDENCE,$EC,$TIMESTAMP,$CPUFREQ_BEFORE,$CPUFREQ_AFTER,$PERF" >> "$CSV"
