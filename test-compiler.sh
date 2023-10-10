#!/usr/bin/env bash

# not sure if you are using
# a compiler correctly?
# this dumps some info

set -euo pipefail

echo "Version:" && $1 --version 2>&1 | head -1

echo "System headers:" && \
    echo \
        | $1 -xc++ -v -E - 2>&1 \
        | awk '/#include <...> search starts here:/{flag=1; next} /End of search list./{flag=0} flag' -
