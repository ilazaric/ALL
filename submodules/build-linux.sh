#!/usr/bin/env bash

set -euo pipefail
set -x

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
OBJ_DIR="$SCRIPT_DIR/objdir/linux"
SRC_DIR="$SCRIPT_DIR/linux"

mkdir -p "$OBJ_DIR"
cp "$SCRIPT_DIR/.config" "$OBJ_DIR/"
make -C "$SRC_DIR" -j $(nproc) O="$OBJ_DIR" bzImage
