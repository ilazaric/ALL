#!/usr/bin/env bash

# -S -i PATH=/bin:/usr/bin LC_ALL=C bash --noprofile --norc

set -euo pipefail

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
OBJ_DIR="$SCRIPT_DIR/objdir/edg"
SRC_DIR="$SCRIPT_DIR/edg-compiler"
# PREFIX="/opt/EDG"

set -x

export EDG_BASE="$SRC_DIR/bases/docker/dev-env/gcc"

if ! [ -d "$OBJ_DIR" ]
then
    cmake -S "$SRC_DIR" -B "$OBJ_DIR" --preset linux-gcc-release -DEDG_CPP_RT_LIBS=linux_x86_64
fi

export EDG_GCC_INCL_SCRAPE="$("$SRC_DIR/dev_tools/bin/edg-scrape-compiler" gcc --lang c++ includes)"
export EDG_GCC_CINCL_SCRAPE="$("$SRC_DIR/dev_tools/bin/edg-scrape-compiler" gcc --lang c includes)"
export EDG_GCC_VER_SCRAPE="$("$SRC_DIR/dev_tools/bin/edg-scrape-compiler" gcc version)"
export EDG_USE_SYSTEM_HEADERS=1

exec choom -n 1000 -- \
     cmake --build "$OBJ_DIR" --parallel 4
