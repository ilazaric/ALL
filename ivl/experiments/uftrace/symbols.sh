#!/usr/bin/env bash

set -euo pipefail

./replay.sh | tr '();{}*/' '       ' | tr ' ' '\n' | sort | uniq -c | sort -n | rev | cut -d ' ' -f 1 | rev | grep -v -e '\[' -e '^[[:digit:]]*$' -e '='
