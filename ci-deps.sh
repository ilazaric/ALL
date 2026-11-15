#!/usr/bin/env bash

set -euo pipefail

DEPS=()

case "$1" in
    "gcc-build")
        DEPS+=(g++-14 bzip2 flex build-essential)
        ;;
    "gcc-test")
        DEPS+=(g++-14 bzip2 flex build-essential python3 python3-pytest tcl dejagnu expect)
        ;;
    "build")
        DEPS+=(libboost-all-dev libgit2-dev libfuse3-dev python3-multiprocess libfftw3-dev libbenchmark-dev)
        DEPS+=(cmake ninja-build)
        # raylib
        DEPS+=(libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl-dev libopengl-dev)
        ;;
    "pragma-once")
        DEPS+=(python3-more-itertools)
        ;;
    "syntax-build")
        DEPS+=(libboost-all-dev libgit2-dev libfuse3-dev python3-multiprocess libfftw3-dev libbenchmark-dev)
        ;;
    *)
        echo "unknown argument: $1"
        exit 1
        ;;
esac

set -x

export DEBIAN_FRONTEND=noninteractive
sudo apt update
sudo apt install -y "${DEPS[@]}"
