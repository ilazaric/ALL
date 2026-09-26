#!/usr/bin/env bash

set -euo pipefail

HOST=debian-perf-1
CORE=3
REPEAT=1
REMOTEDIR="/tmp/ivl-perf-run"
CLEANED=0
QUOTED="$(printf '%q' "$REMOTEDIR")"

# set -x

# TODO: lock host so i cant accidentally run two of these at same time

# TODO: copying of shared libs needed (ldd to find them, LD_LIBRARY_PATH to force them)

function print_help() {
    echo 'perf-run -- script to execute on remote host configured for low variance

usage: ivl script perf-run [options...] -- command...

options:
  --copy SRC
    copy local file SRC to remote directory owned by perf-run before running command
    since working directory will be the perf-run directory, the file is accessible via filename

  --copy-to SRC DST
    copy local file SRC to remote location DST before running command

  --repeat N
    remote job will be executed N times, defaults to 1

  --help
    print this message and exit
'
}

function init_remote() {
    if [ "$CLEANED" -eq 1 ]
    then
        return
    fi
    CLEANED=1
    ssh "$HOST" -- "rm -rf $QUOTED; mkdir -p $QUOTED"
}

# TODO: maybe make the body case/esac instead of ifs
while true
do
    if [ "$#" -eq 0 ]
    then
        echo "ERROR: command should be delimited from options (even if no options used) with `--`"
        echo
        print_help
        exit 1
    fi
    if [ "$1" == "--" ]
    then
        shift
        break
    fi
    if [ "$1" == "--help" ]
    then
        print_help
        exit 0
    fi
    if [ "$1" == "--copy" ]
    then
        shift
        SRC="$1"
        shift
        init_remote
        # TODO: maybe rsync instead, we could support dirs that way too
        scp "$SRC" "$HOST:$REMOTEDIR/$(basename "$SRC")"
        continue
    fi
    if [ "$1" == "--copy-to" ]
    then
        shift
        SRC="$1"
        shift
        DST="$1"
        shift
        init_remote
        # TODO: maybe rsync instead, we could support dirs that way too
        scp "$SRC" "$HOST:$DST"
        continue
    fi
    if [ "$1" == "--repeat" ]
    then
        shift
        REPEAT="$1"
        shift
        continue
    fi
    echo "unrecognized option: $1" > /dev/stderr
    echo
    print_help
    exit 1
done

init_remote
ssh << EOF "$HOST" "cat > $QUOTED/script.sh"
#/usr/bin/env bash

cd $QUOTED

for ((i = 0; i < $REPEAT; ++i))
do
    echo "RUN #\$i ..."
    echo
    {
    time \\
        taskset -c "$CORE" \\
        env -i \\
        LC_ALL=C \\
        PATH="/opt/GCC-release/bin:/bin:/usr/bin" \\
        $(printf '%q ' "$@")
    } 2>&1
    echo
    echo "EXIT CODE: \$?"
    echo
    echo "RUN #\$i done"
    echo
done
EOF

ssh "$HOST" -- bash "$REMOTEDIR/script.sh"
