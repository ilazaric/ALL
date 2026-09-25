#!/usr/bin/env bash

set -euo pipefail

HOST=debian-perf-1
CORE=3

set -x

# TODO: lock host so i cant accidentally run two of these at same time

# TODO: ship files needed for run

ssh "$HOST" -- \
    time \
    LC_ALL=C \
    PATH="/opt/GCC-release/bin:/bin:/usr/bin" \
    taskset -c "$CORE" \
    env \
    "$@"
