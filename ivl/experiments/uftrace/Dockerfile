FROM ubuntu:24.04

RUN apt-get update && apt-get install -y gcc gdb emacs
RUN apt-get install -y g++ uftrace

COPY foo.c /
COPY ubsan-bug.cpp /
COPY .emacs /

ENV LC_ALL=C
ENV LANG=en_US.UTF-8

RUN gcc -Og foo.c -o foo
