#!/usr/bin/env bash

set -euo pipefail
set -x

echo 2500000 | tee /sys/devices/system/cpu/cpu3/cpufreq/scaling_min_freq
echo 2500000 | tee /sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq
echo performance | tee /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor
echo 1 | tee /sys/devices/system/cpu/intel_pstate/no_turbo
