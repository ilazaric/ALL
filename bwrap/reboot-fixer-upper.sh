#!/usr/bin/env bash

set -euo pipefail
set -x

host="$1" ; shift

for core in 0 $(eval "echo {$(ssh $host -- cat /sys/devices/system/cpu/isolated | sed 's/-/../g')}")
do
    ssh root@$host -- "echo performance > /sys/devices/system/cpu/cpu${core}/cpufreq/scaling_governor"
    ssh root@$host -- "echo 2500000 > /sys/devices/system/cpu/cpufreq/policy${core}/scaling_max_freq"
done

# for cpu in /sys/devices/system/cpu/cpu{1,2,3}/cpufreq/scaling_governor; do   echo performance | sudo tee $cpu; done
# ssh root@$host -- 'echo 0 > /proc/sys/kernel/randomize_va_space'
# ssh root@$host -- 'systemctl start mydaemon.socket'
