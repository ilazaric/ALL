#!/usr/bin/env bash

set -euo pipefail
set -x

echo 2500000 > /sys/devices/system/cpu/cpu3/cpufreq/scaling_min_freq
echo 2500000 > /sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq
echo performance > /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor
