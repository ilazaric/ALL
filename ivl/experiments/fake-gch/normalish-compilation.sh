#!/usr/bin/env bash

source common.sh || exit 1

time g++ -xc++ -Wl,-rpath=/opt/GCC/lib64 -std=c++26 -O3 -include main1.cpp /dev/null -o main1
