#!/usr/bin/env bash

set -euo pipefail

unshare -UrmpniuCT --fork ./try2.sh "$@"
