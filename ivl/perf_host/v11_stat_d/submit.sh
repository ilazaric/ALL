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

[ "$(cat "$OUTPUT" | wc -l)" -eq 16 ]

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

PERF="$(output_line 5 "perf:task-clock")"
PERF="$PERF,$(output_line 6 "perf:context-switches")"
PERF="$PERF,$(output_line 7 "perf:cpu-migrations")"
PERF="$PERF,$(output_line 8 "perf:page-faults")"
PERF="$PERF,$(output_line 9 "perf:cycles")"
PERF="$PERF,$(output_line 10 "perf:instructions")"
PERF="$PERF,$(output_line 11 "perf:branches")"
PERF="$PERF,$(output_line 12 "perf:branch-misses")"
PERF="$PERF,$(output_line 13 "perf:L1-dcache-loads")"
PERF="$PERF,$(output_line 14 "perf:L1-dcache-load-misses")"
PERF="$PERF,$(output_line 15 "perf:LLC-loads")"
PERF="$PERF,$(output_line 16 "perf:LLC-load-misses")"

echo "$DURATION,$EVIDENCE,$EC,$TIMESTAMP,$CPUFREQ_BEFORE,$CPUFREQ_AFTER,$PERF" >> "$CSV"
