#!/usr/bin/env bash

unshare \
    --user \
    --map-root-user \
    --uts \
    --time \
    --net \
    --cgroup \
    --mount \
    --fork \
    --pid \
    --mount-proc \
    "$@"
