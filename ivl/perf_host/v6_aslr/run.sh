#!/usr/bin/env bash

echo "cpufreq_before: $(cat /sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq)"
taskset -c 3 /tmp/benchmark
echo "cpufreq_after: $(cat /sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq)"
