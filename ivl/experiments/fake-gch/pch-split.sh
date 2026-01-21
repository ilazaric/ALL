#!/usr/bin/env bash

source common.sh || exit 1

time g++ -xc++-header -Wl,-rpath=/opt/GCC/lib64 -std=c++26 -O3 heavy-header.hpp -c
time g++ -xc++ -Wl,-rpath=/opt/GCC/lib64 -std=c++26 -O3 main1.cpp -o main1
