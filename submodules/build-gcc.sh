#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
OBJ_DIR="$SCRIPT_DIR/objdir/gcc"
SRC_DIR="$SCRIPT_DIR/gcc"

INSTALL=0
CLEAN_FIRST=0
ARG_ERRORS=0

function print_help() {
    echo './build-gcc.sh -- script to compile g++

options:
  -j NUM
    number of processes allowed to run in parallel, defaults to $(nproc)

  --install
    run `make install` after successful build

  --clean-first
    fully purge objdir before building

  --help
    print this message and exit
'
}

while [ "$#" -ne 0 ]
do
    OPTION="$1"
    shift
    case "$OPTION" in
        "-j")
            PARALLELISM="$1"
            shift
            ;;
        "--install")
            INSTALL=1
            ;;
        "--clean-first")
            CLEAN_FIRST=1
            ;;
        "--help")
            print_help
            ARG_ERRORS=1
            ;;
        *)
            echo "unknown argument: $OPTION"
            ARG_ERRORS=1
            ;;
    esac
done

if [ $ARG_ERRORS -eq 1 ]
then
    exit 1
fi

set -x

if [ $CLEAN_FIRST -eq 1 ]
then
    echo "purging objdir ..."
    rm -rf "$OBJ_DIR"
fi

if [ -z ${PARALLELISM+x} ]
then
    PARALLELISM="$(nproc)"
    echo "defaulting parallelism to $PARALLELISM"
fi

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

make -j "$PARALLELISM"

if [ $INSTALL -eq 1 ]
then
    echo "installing ..."
    make install
fi
