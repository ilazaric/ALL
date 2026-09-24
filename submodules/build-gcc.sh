#!/usr/bin/env -S -i PATH=/bin:/usr/bin LC_ALL=C bash --noprofile --norc

set -euo pipefail

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
OBJ_DIR="$SCRIPT_DIR/objdir/gcc"
SRC_DIR="$SCRIPT_DIR/gcc"
PREFIX="/opt/GCC"

INSTALL=0
CLEAN_FIRST=0
ARG_ERRORS=0
CHECKING=yes
EXTEND_PATHS=1

function print_help() {
    echo './build-gcc.sh -- script to compile g++

options:
  -j NUM
    number of processes allowed to run in parallel, defaults to $(nproc)

  --install
    run `make install` after successful build

  --clean-first
    fully purge objdir before building

  --checking <string>
    argument for `--enable-checking=`
    default: yes
    description of `--enable-checking` from gcc/configure:
      --enable-checking[=LIST]
                          enable expensive run-time checks. With LIST, enable
                          only specific categories of checks. Categories are:
                          yes,no,all,none,release. Flags are:
                          assert,df,extra,fold,gc,gcac,gimple,misc,
                          rtlflag,rtl,runtime,tree,valgrind,types

  --dont-extend-paths-with-checking
    usually OBJ_DIR and PREFIX get extended with "-$CHECKING"
    this flag kills this extension
    only really relevant for CI

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
        "--dont-extend-paths-with-checking")
            EXTEND_PATHS=0
            ;;
        "--clean-first")
            CLEAN_FIRST=1
            ;;
        "--checking")
            CHECKING="$1"
            shift
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

which gcc g++

if [ $EXTEND_PATHS -eq 1 ]
then
   OBJ_DIR="$OBJ_DIR-$CHECKING"
   PREFIX="$PREFIX-$CHECKING"
fi

echo "OBJ_DIR = $OBJ_DIR"
echo "PREFIX  = $PREFIX"

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
    mkdir -p "$OBJ_DIR"
    cd "$OBJ_DIR"
    "$SRC_DIR/configure"                 \
        --disable-multilib               \
        --prefix="$PREFIX"               \
        --enable-languages=c,c++         \
        --enable-libstdcxx-debug         \
        --enable-libstdcxx-backtrace     \
        --disable-bootstrap              \
        --disable-libvtv                 \
        --disable-libssp                 \
        --disable-libffi                 \
        --with-system-zlib               \
        --without-isl                    \
        --enable-checking="$CHECKING"
else
    cd "$OBJ_DIR"
fi

# TODO: would be better to not unset it in first place
DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/$(id -u)/bus" \
  systemd-run --user --scope \
  -p OOMPolicy=kill \
  choom -n 1000 -- \
  env -u DBUS_SESSION_BUS_ADDRESS -u INVOCATION_ID -- \
  make -j "$PARALLELISM"

if [ $INSTALL -eq 1 ]
then
    echo "installing ..."
    make install
fi
