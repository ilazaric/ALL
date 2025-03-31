#!/usr/bin/env bash

# arguments: command to execute, alongside flags
# example: /usr/bin/bash --norc --noprofile
# first argument has to be a (relative or absolute) path to the program

# bwrap="/home/ilazaric/repos/ALL/bwrap/bubblewrap-0.11.0/builddir/bwrap"

set -euo pipefail
set -x

# ./generate-bpf-file.py
# exec 3< good-syscalls.bpf

# program="$1" ; shift

systemd-run \
    --user --scope \
    -p MemoryAccounting=yes \
    -p MemorySwapMax=0 \
    -p MemoryMax=500M \
bwrap \
    --die-with-parent \
    --clearenv \
    --unshare-all \
    --unshare-user \
    --unshare-cgroup \
    --dir / \
    --dir /bin \
    --ro-bind /lib /lib \
    --ro-bind /lib64 /lib64 \
    --ro-bind /usr/bin/bash /bin/bash \
    --ro-bind /usr/lib/linux-tools/6.11.0-21-generic/../../linux-hwe-6.11-tools-6.11.0-21/perf /bin/perf \
    --ro-bind taskset-and-exec /bin/taskset-and-exec \
    --ro-bind drop-syscalls /bin/drop-syscalls \
    --ro-bind tree-sum/recursive-bench /bin/bench \
    --cap-drop ALL \
    --proc /proc \
    --chdir / \
    -- "$@" || echo "!!! FAILURE !!! EXIT_CODE: [$?]"

# killing access to a bunch of syscalls:
#    --seccomp 3 \

#    --ro-bind "$(realpath "$program")" /bin/program \
