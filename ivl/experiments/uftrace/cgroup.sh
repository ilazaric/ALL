#!/usr/bin/env bash

set -euo pipefail

BASE="/sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service"
DIR="$BASE/ubsan-bug"

function error() {
    echo "ERROR:" "$@" > /dev/stderr
    exit 1
}

function create_and_launch() {
    mkdir "$DIR"
    echo "executing under cgroup $DIR ..."
    echo "$$" > "$DIR/cgroup.procs"
    echo "$DIR/memory.peak =="
    cat "$DIR/memory.peak"
    exec "$@"
    exit 1 # not reachable
}

[ -d "$BASE" ] || error "missing base directory: $BASE"

if ! [ -e "$DIR" ]
then
    create_and_launch "$@"
    exit 1 # not reachable
fi

[ -d "$DIR" ] || error "cgroup path not a directory: $DIR"

rmdir "$DIR"
create_and_launch "$@"
exit 1 # not reachable
