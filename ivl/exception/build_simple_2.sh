#!/usr/bin/env bash

g++ simple_2.cpp -c -o simple_2.o
g++ simple_2_2.cpp -c -o simple_2_2.o
g++ simple_2.o simple_2_2.o -o simple_2
