#!/usr/bin/env bash

set -euo pipefail
set -x

DATADIR="$(dirname "$(realpath "$0")")"
BENCHMARK="$DATADIR/../binary_tree_traversal_benchmark"
HOST="debian-perf-1"
OUTPUT="$DATADIR/output"
CSV="$DATADIR/data.csv"

[ -f "$BENCHMARK" ]
[ -f "$CSV" ]

TIMESTAMP="$(date +%s%N)"

rsync "$BENCHMARK" "$HOST:/tmp/benchmark"
ssh "$HOST" -- /tmp/benchmark &> "$OUTPUT" && EC="$?" || EC="$?"

[ "$(cat "$OUTPUT" | wc -l)" -eq "2" ]

DURATION="$(head -1 "$OUTPUT" | cut -d ' ' -f 2- )"
EVIDENCE="$(tail -1 "$OUTPUT" | cut -d ' ' -f 2- )"

echo "$DURATION,$EVIDENCE,$EC,$TIMESTAMP" >> "$CSV"
