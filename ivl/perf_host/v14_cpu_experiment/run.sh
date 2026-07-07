#!/usr/bin/env bash

set -euo pipefail

# set -x

rm -rf /tmp/perf-output

echo -n "cpufreq_before: "
cat /sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq

# taken from `man perf-stat` , look for `--control`
ctl_dir=/tmp/

ctl_fifo=${ctl_dir}perf_ctl.fifo
if test -p ${ctl_fifo}
then
    unlink ${ctl_fifo}
fi
mkfifo ${ctl_fifo}

ctl_ack_fifo=${ctl_dir}perf_ctl_ack.fifo
if test -p ${ctl_ack_fifo}
then
    unlink ${ctl_ack_fifo}
fi
mkfifo ${ctl_ack_fifo}

perf stat \
     -j \
     -o /tmp/perf-output \
    -D -1 --control fifo:${ctl_fifo},${ctl_ack_fifo} \
    -d \
    taskset -c 3 /tmp/delayed_exec $ctl_fifo $ctl_ack_fifo /tmp/benchmark

echo -n "cpufreq_after: "
cat /sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq

jq -r '"perf:" + .event + ": " + ."counter-value"' /tmp/perf-output
rm /tmp/perf-output
unlink ${ctl_ack_fifo}
unlink ${ctl_fifo}
