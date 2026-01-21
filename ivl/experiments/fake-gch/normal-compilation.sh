#!/usr/bin/env bash

source common.sh || exit 1

time g++ -Wl,-rpath=/opt/GCC/lib64 -std=c++26 -O3 main1.cpp -o main1
