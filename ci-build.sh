#!/usr/bin/env bash

set -euo pipefail

set -x

bad=(
    /experiments
    /spoj
)

cd "$(dirname "$(realpath "0")")"

exec \
./build.py --report-durations --keep-going --verbose --jobs $(nproc) \
    $( comm -23 \
       <( find ivl -maxdepth 1 -mindepth 1 -type d | cut -c 4- | sort ) \
       <( printf '%s\n' "${bad[@]}" | sort ) )
