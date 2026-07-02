#!/usr/bin/env bash

set -euo pipefail
set -x

DATADIR="$(dirname "$(realpath "$0")")"
BENCHMARK="$DATADIR/../binary_tree_traversal_benchmark"
HOST="debian-perf-1"
OUTPUT="$DATADIR/output"
CSV="$DATADIR/data.csv"
RUNNER="$DATADIR/run.sh"

[ -f "$BENCHMARK" ]
[ -f "$CSV" ]
[ -f "$RUNNER" ]

TIMESTAMP="$(date +%s%N)"

rsync -a "$BENCHMARK" "$HOST:/tmp/benchmark"
rsync -a "$RUNNER" "$HOST:/tmp/runner"
ssh "$HOST" -- "/tmp/runner" &> "$OUTPUT" && EC="$?" || EC="$?"

[ "$(cat "$OUTPUT" | wc -l)" -eq 4 ]

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

echo "$DURATION,$EVIDENCE,$EC,$TIMESTAMP,$CPUFREQ_BEFORE,$CPUFREQ_AFTER" >> "$CSV"
