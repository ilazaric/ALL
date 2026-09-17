#!/usr/bin/env bash

set -euo pipefail

FROM="$1"
shift
TO="$1"
shift

git grep -l -e "$FROM" "$@" | ivl script replace "$FROM" "$TO"
