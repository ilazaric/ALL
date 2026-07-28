#!/usr/bin/env bash

set -euo pipefail
set -x

# COMMIT="$(git rev-parse HEAD:submodules/gcc)"
SCRIPT_DIR="$(dirname "$(realpath "$0")")"
OBJ_DIR="$SCRIPT_DIR/objdir/gcc"
SRC_DIR="$SCRIPT_DIR/gcc"

cd "$SRC_DIR"
./contrib/download_prerequisites

if ! [ -d "$OBJ_DIR" ]
then
    mkdir "$OBJ_DIR"
    cd "$OBJ_DIR"
    "$SRC_DIR/configure"                 \
        --disable-multilib               \
        --prefix="/opt/GCC"              \
        --enable-languages=c,c++         \
        --enable-libstdcxx-debug         \
        --enable-libstdcxx-backtrace     \
        --disable-bootstrap              \
        --disable-libvtv                 \
        --disable-libssp                 \
        --disable-libffi                 \
        --with-system-zlib               \
        --without-isl                    \
        --enable-checking=release
else
    cd "$OBJ_DIR"
fi

make -j $(nproc)

if [ "$#" -ge 1 ] && [ "$1" == "--install" ]
then
    echo "installing ..."
    make install
fi
