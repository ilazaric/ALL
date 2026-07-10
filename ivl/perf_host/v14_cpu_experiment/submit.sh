#!/usr/bin/env bash

set -euo pipefail
# set -x

DATADIR="$(dirname "$(realpath "$0")")"
BENCHMARK="$DATADIR/../binary_tree_traversal_benchmark"
HOST="debian-perf-1"
OUTPUT="$DATADIR/output"
CSV="$DATADIR/data.csv"
RUNNER="$DATADIR/run.sh"
DELAYED_EXEC="$DATADIR/../delayed_exec"
HOST_INIT="$DATADIR/host-init.sh"

[ -x "$BENCHMARK" ]
[ -f "$CSV" ]
[ -x "$RUNNER" ]
[ -x "$DELAYED_EXEC" ]
[ -x "$HOST_INIT" ]

# has header?
[ "$(cat "$CSV" | wc -l)" -ge 1 ]

rsync -a "$HOST_INIT" "$HOST:/tmp/host_init"
rsync -a "$BENCHMARK" "$HOST:/tmp/benchmark"
rsync -a "$RUNNER" "$HOST:/tmp/runner"
rsync -a "$DELAYED_EXEC" "$HOST:/tmp/delayed_exec"
ssh "$HOST" -- 'echo foobar | su -c /tmp/host_init'

function output_line() {
    LINE="$(head "-$1" $OUTPUT | tail -1)"
    [ "$(echo "$LINE" | cut -d ' ' -f 1)" = "$2:" ]
    [ "$(echo "$LINE" | cut -d ' ' -f 3- )" = "" ]
    echo "$LINE" | cut -d ' ' -f 2
}

function header() {
    cut -d ' ' -f 1 "$OUTPUT" | rev | cut -c 2- | rev | tr '\n' ','
    echo 'timestamp,exit_code'
}

function values() {
    cut -d ' ' -f 2- "$OUTPUT" | tr '\n' ','
    echo "$TIMESTAMP,$EC"
}

# header
# exit 0

HEADER="$(head -1 "$CSV")"

for ITER in $(seq 1 "$1")
do
    echo "ITER=$ITER"

    TIMESTAMP="$(date +%s%N)"

    ssh "$HOST" -- "/tmp/runner" > "$OUTPUT" && EC="$?" || EC="$?"

    [ "$(header)" == "$HEADER" ]

    values >> "$CSV"
done
