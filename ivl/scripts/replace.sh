#!/usr/bin/env bash

set -euo pipefail

parallel "sed -i 's|$1|$2|g' {}" < /dev/stdin
