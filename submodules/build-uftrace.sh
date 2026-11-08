#!/usr/bin/env -S -i PATH=/bin:/usr/bin LC_ALL=C bash --noprofile --norc

set -euo pipefail

export HOME="$(getent passwd "$(id -u)" | cut -d: -f6)"
export XDG_RUNTIME_DIR="/run/user/$(id -u)"

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
OBJ_DIR="$SCRIPT_DIR/objdir/uftrace"
SRC_DIR="$SCRIPT_DIR/uftrace"
PREFIX="/opt/uftrace"

INSTALL=0
CLEAN_FIRST=0
ARG_ERRORS=0
PURGE_OLD_INSTALL=0

function print_help() {
    echo './build-uftrace.sh -- script to compile uftrace

options:
  -j NUM
    number of processes allowed to run in parallel, defaults to $(nproc)

  --install
    run `make install` after successful build, fail if install already exists

  --reinstall
    run `make install` after successful build, but also remove old install if exists

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
        "--reinstall")
            INSTALL=1
            PURGE_OLD_INSTALL=1
            ;;
        "--clean-first")
            CLEAN_FIRST=1
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

set +x
echo
echo "general info:"
echo "OBJ_DIR              = $OBJ_DIR"
echo "PREFIX               = $PREFIX"
echo "CC                   = $(which gcc)"
echo "CXX                  = $(which g++)"
echo
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

if ! [ -d "$OBJ_DIR" ]
then
    mkdir -p "$OBJ_DIR"
    cd "$OBJ_DIR"
    "$SRC_DIR/configure" --prefix="$PREFIX"
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

if [ $PURGE_OLD_INSTALL -eq 1 ]
then
    echo "purging old install ..."
    rm -rf "$PREFIX"
fi

if [ $INSTALL -eq 1 ]
then
    echo "installing ..."
    if [ -e "$PREFIX" ]
    then
        echo "error, cannot install as directory already exists: $PREFIX"
        echo "note: consider using --reinstall"
        exit 1
    fi
    make install
fi
