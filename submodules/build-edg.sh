#!/usr/bin/env bash

# -S -i PATH=/bin:/usr/bin LC_ALL=C bash --noprofile --norc

set -euo pipefail

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
OBJ_DIR="$SCRIPT_DIR/objdir/edg"
SRC_DIR="$SCRIPT_DIR/edg-compiler"
# PREFIX="/opt/EDG"

COMPILER="gcc"
BUILD_TYPE="release"
CLEAN_FIRST=0
ARG_ERRORS=0

function print_help() {
    echo './build-edg.sh -- script to compile edg

options:
  --compiler <string>
    which compiler to use, as found from PATH
    valid options: gcc, clang
    default: gcc

  --type <string>
    build type, for cmake preset
    valid options: release, debug
    default: release

  --clean-first
    fully purge objdir before building

  -- <args>...
    consumes `--`, passes everything else to `cmake --build`

  --help
    print this message and exit
'
}

while [ "$#" -ne 0 ]
do
    OPTION="$1"
    shift
    case "$OPTION" in
        "--compiler")
            COMPILER="$1"
            shift
            ;;
        "--type")
            BUILD_TYPE="$1"
            shift
            ;;
        "--clean-first")
            CLEAN_FIRST=1
            ;;
        "--")
            break;
            ;;
        "--help")
            print_help
            exit 0
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

[[ "$COMPILER" == "gcc" || "$COMPILER" == "clang" ]]
[[ "$BUILD_TYPE" == "release" || "$BUILD_TYPE" == "debug" ]]

which "$COMPILER"

if [ "$COMPILER" != "gcc" ]
then
    OBJ_DIR="${OBJ_DIR}-${COMPILER}"
fi

if [ "$BUILD_TYPE" != "release" ]
then
    OBJ_DIR="${OBJ_DIR}-${BUILD_TYPE}"
fi

echo "OBJ_DIR = $OBJ_DIR"

if [ $CLEAN_FIRST -eq 1 ]
then
    echo "purging objdir ..."
    rm -rf "$OBJ_DIR"
fi

CXX_FLAGS=""
if [ "$COMPILER" == "clang" ]
then
    CXX_FLAGS="-stdlib=libc++ -DHOST_COMPILER_SUPPORTS_BFLOAT16=FALSE"
fi

export EDG_BASE="$SRC_DIR/bases/docker/dev-env/$COMPILER"
EDG_COMPILER_INCL_SCRAPE="$("$SRC_DIR/dev_tools/bin/edg-scrape-compiler" "$COMPILER" --lang c++ includes)"
EDG_COMPILER_CINCL_SCRAPE="$("$SRC_DIR/dev_tools/bin/edg-scrape-compiler" "$COMPILER" --lang c includes)"
EDG_COMPILER_VER_SCRAPE="$("$SRC_DIR/dev_tools/bin/edg-scrape-compiler" "$COMPILER" version)"
# too lazy to figure out better way
export EDG_GCC_INCL_SCRAPE="$EDG_COMPILER_INCL_SCRAPE"
export EDG_GCC_CINCL_SCRAPE="$EDG_COMPILER_CINCL_SCRAPE"
export EDG_GCC_VER_SCRAPE="$EDG_COMPILER_VER_SCRAPE"
export EDG_CLANG_INCL_SCRAPE="$EDG_COMPILER_INCL_SCRAPE"
export EDG_CLANG_CINCL_SCRAPE="$EDG_COMPILER_CINCL_SCRAPE"
export EDG_CLANG_VER_SCRAPE="$EDG_COMPILER_VER_SCRAPE"
export EDG_INCLDIR="$EDG_COMPILER_INCL_SCRAPE"
export EDG_CINCLDIR="$EDG_COMPILER_CINCL_SCRAPE"
export EDG_USE_SYSTEM_HEADERS=1

if ! [ -d "$OBJ_DIR" ]
then
    cmake \
        -S "$SRC_DIR" \
        -B "$OBJ_DIR" \
        --preset "linux-${COMPILER}-${BUILD_TYPE}" \
        -DEDG_CPP_RT_LIBS=linux_x86_64 \
        -DCMAKE_COLOR_DIAGNOSTICS=ON \
        -DCMAKE_CXX_FLAGS="$CXX_FLAGS"
fi

# function syntax_run() {
#     /opt/GCC-release/bin/g++ -DBACK_END_IS_CP_GEN_BE=1 -DBACK_END_IS_C_GEN_BE=0 -DUSE_CMAKE_DEFINES -I/home/ilazaric/repos/ALL/submodules/objdir/edg-dbg/src/includes -g -std=c++14 -fdiagnostics-color=always -g3 -Wall -Wwrite-strings -Wformat-security -pedantic -Werror=shadow -Wimplicit-fallthrough -Wconversion -Wsign-conversion -fsanitize=address -fno-exceptions -fno-unwind-tables -fno-rtti -fsyntax-only -x c++ "$1"
# }

# function syntax_test() {
#     syntax_run "$1" &> /dev/null && echo " ... pass" || echo " !!! FAIL"
# }

# export -f syntax_run
# export -f syntax_test

# # syntax_run "$SRC_DIR"/src/header_util.h
# # exit 0

# set +x
# echo "$SRC_DIR"/src/*.h | tr ' ' '\n' | parallel -j 10 'echo -n {}; syntax_test {}'
# exit 0

exec choom -n 1000 -- \
     cmake --build "$OBJ_DIR" "$@" # --parallel 4

# /opt/GCC-release/bin/g++ -DBACK_END_IS_CP_GEN_BE=1 -DBACK_END_IS_C_GEN_BE=0 -DUSE_CMAKE_DEFINES -I/home/ilazaric/repos/ALL/submodules/objdir/edg-dbg/src/includes -g -std=c++14 -fdiagnostics-color=always -g3 -Wall -Wwrite-strings -Wformat-security -pedantic -Werror=shadow -Wimplicit-fallthrough -Wconversion -Wsign-conversion -fsanitize=address -fno-exceptions -fno-unwind-tables -fno-rtti -x c++ -o src/cmake/cpfe-cp/CMakeFiles/cpfe-cp.dir/__/__/ivl.c.o -c /home/ilazaric/repos/ALL/submodules/edg-compiler/src/ivl.c
