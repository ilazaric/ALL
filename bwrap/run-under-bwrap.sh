#!/usr/bin/env bash

# arguments: command to execute, alongside flags
# example: /usr/bin/bash --norc --noprofile
# first argument has to be a (relative or absolute) path to the program

# bwrap="/home/ilazaric/repos/ALL/bwrap/bubblewrap-0.11.0/builddir/bwrap"

set -euo pipefail
set -x

# ./generate-bpf-file.py
# exec 3< good-syscalls.bpf

program="$1" ; shift

systemd-run \
    --user --scope \
    -p MemoryAccounting=yes \
    -p MemorySwapMax=0 \
    -p MemoryMax=100M \
bwrap \
    --die-with-parent \
    --clearenv \
    --unshare-all \
    --unshare-user \
    --unshare-cgroup \
    --dir / \
    --dir /bin \
    --ro-bind /usr/bin/bash /bin/bash \
    --ro-bind /lib /lib \
    --ro-bind /lib64 /lib64 \
    --ro-bind "$(realpath "$program")" /bin/program \
    --cap-drop ALL \
    --proc /proc \
    --chdir / \
    -- /bin/program "$@" || echo "!!! FAILURE !!! EXIT_CODE: [$?]"

# killing access to a bunch of syscalls:
#    --seccomp 3 \
