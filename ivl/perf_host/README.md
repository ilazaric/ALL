# low variance benchmarking
11;rgb:0000/0000/0000
on a crappy host i installed minimal debian, no X11  
the host is intended to be a low variance environment for performance testing  

## host properties

```
ilazaric@debian-perf-1:~$ lscpu
Architecture:                x86_64
  CPU op-mode(s):            32-bit, 64-bit
  Address sizes:             39 bits physical, 48 bits virtual
  Byte Order:                Little Endian
CPU(s):                      4
  On-line CPU(s) list:       0-3
Vendor ID:                   GenuineIntel
  Model name:                Intel(R) Core(TM) i5-6500T CPU @ 2.50GHz
    CPU family:              6
    Model:                   94
    Thread(s) per core:      1
    Core(s) per socket:      4
    Socket(s):               1
    Stepping:                3
    CPU(s) scaling MHz:      27%
    CPU max MHz:             3100.0000
    CPU min MHz:             800.0000
    BogoMIPS:                4999.90
    Flags:                   fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop
                             _tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl smx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fau
                             lt epb pti ssbd ibrs ibpb stibp fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid mpx rdseed adx smap clflushopt intel_pt xsaveopt xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp m
                             d_clear flush_l1d arch_capabilities
Caches (sum of all):         
  L1d:                       128 KiB (4 instances)
  L1i:                       128 KiB (4 instances)
  L2:                        1 MiB (4 instances)
  L3:                        6 MiB (1 instance)
NUMA:                        
  NUMA node(s):              1
  NUMA node0 CPU(s):         0-3
Vulnerabilities:             
  Gather data sampling:      Vulnerable: No microcode
  Indirect target selection: Not affected
  Itlb multihit:             KVM: Mitigation: VMX unsupported
  L1tf:                      Mitigation; PTE Inversion
  Mds:                       Mitigation; Clear CPU buffers; SMT disabled
  Meltdown:                  Mitigation; PTI
  Mmio stale data:           Mitigation; Clear CPU buffers; SMT disabled
  Reg file data sampling:    Not affected
  Retbleed:                  Mitigation; IBRS
  Spec rstack overflow:      Not affected
  Spec store bypass:         Mitigation; Speculative Store Bypass disabled via prctl
  Spectre v1:                Mitigation; usercopy/swapgs barriers and __user pointer sanitization
  Spectre v2:                Mitigation; IBRS; IBPB conditional; STIBP disabled; RSB filling; PBRSB-eIBRS Not affected; BHI Not affected
  Srbds:                     Mitigation; Microcode
  Tsa:                       Not affected
  Tsx async abort:           Mitigation; TSX disabled
  Vmscape:                   Mitigation; IBPB before exit to userspace
ilazaric@debian-perf-1:~$ lstopo
Machine (7828MB total)
  Package L#0
    NUMANode L#0 (P#0 7828MB)
    L3 L#0 (6144KB)
      L2 L#0 (256KB) + L1d L#0 (32KB) + L1i L#0 (32KB) + Core L#0 + PU L#0 (P#0)
      L2 L#1 (256KB) + L1d L#1 (32KB) + L1i L#1 (32KB) + Core L#1 + PU L#1 (P#1)
      L2 L#2 (256KB) + L1d L#2 (32KB) + L1i L#2 (32KB) + Core L#2 + PU L#2 (P#2)
      L2 L#3 (256KB) + L1d L#3 (32KB) + L1i L#3 (32KB) + Core L#3 + PU L#3 (P#3)
  HostBridge
    PCI 00:02.0 (VGA)
    PCI 00:17.0 (SATA)
      Block(Disk) "sda"
    PCI 00:1f.6 (Ethernet)
      Net "eno1"
ilazaric@debian-perf-1:~$ cat /proc/cmdline
BOOT_IMAGE=/boot/vmlinuz-6.12.94+deb13-amd64 root=UUID=2f5fb668-5b04-4469-9731-f83447e7f283 ro quiet
ilazaric@debian-perf-1:~$ lsb_release -a
No LSB modules are available.
Distributor ID:	Debian
Description:	Debian GNU/Linux 13 (trixie)
Release:	13
Codename:	trixie
ilazaric@debian-perf-1:~$ free -h
               total        used        free      shared  buff/cache   available
Mem:           7.6Gi       423Mi       6.7Gi       1.1Mi       816Mi       7.2Gi
Swap:          6.2Gi          0B       6.2Gi
```

## benchmark: `binary_tree_traversal_benchmark`

a rather simple synthetic benchmark that crawls over a binary tree

the benchmark outputs two lines
```
duration: <nanoseconds>
evidence: <proof-of-correctness>
```

duration ought to be ~0.2s  
evidence must be `49954166378`

## evaluation format

each suite of experiments will have an associated `data.csv`  
the columns will at least have `duration` , `evidence` , `exit_code` , `timestamp` (when was the experiment ran)  
it is permitted to have more columns

## system experiments

we try out multiple system / evaluation configurations,  
submit the benchmark thousands of times,  
and consider the data, try to minimize variance  

### v1: default

literally nothing, we just copy the benchmark and run it, not even `taskset`

generated 1k samples in `v1_default/data.csv`

`CV = 1.29%`

### v2: isolcpus/taskset

added `isolcpus=3` to kernel parameters

```
ilazaric@debian-perf-1:~$ cat /etc/default/grub | grep DEF
GRUB_DEFAULT=0
GRUB_CMDLINE_LINUX_DEFAULT="quiet isolcpus=3"
ilazaric@debian-perf-1:~$ cat /proc/cmdline
BOOT_IMAGE=/boot/vmlinuz-6.12.94+deb13-amd64 root=UUID=2f5fb668-5b04-4469-9731-f83447e7f283 ro quiet isolcpus=3
```

added 'taskset -c 3' prefix to submit script

`CV = 1.72%`

CV increased for some reason , the benchmark is quite fast so maybe it didnt have time to context switch

### v3: irq affinity

added `irqaffinity=0,1,2` to kernel params

```
ilazaric@debian-perf-1:~$ cat /proc/cmdline
BOOT_IMAGE=/boot/vmlinuz-6.12.94+deb13-amd64 root=UUID=2f5fb668-5b04-4469-9731-f83447e7f283 ro quiet isolcpus=3 irqaffinity=0,1,2
ilazaric@debian-perf-1:~$ cat /proc/irq/0/smp_affinity 
7
```

`CV = 1.86%`

once again increased, which is bad, but unsure if actually bad

### v4: cpu frequency

make cpu frequency less jumpy, probably can do more here by changing the governor but good start

```
root@debian-perf-1:/home/ilazaric# echo 2500000 > /sys/devices/system/cpu/cpu3/cpufreq/scaling_min_freq
root@debian-perf-1:/home/ilazaric# echo 2500000 > /sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq
```

```
ilazaric@debian-perf-1:~$ for f in  /sys/devices/system/cpu/cpu3/cpufreq/* ; do ls -lah $f ; cat $f ; echo ; done
-r--r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/affected_cpus
3

-r--r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/base_frequency
2500000

-r--r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/cpuinfo_max_freq
3100000

-r--r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/cpuinfo_min_freq
800000

-r--r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/cpuinfo_transition_latency
0

-r--r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/energy_performance_available_preferences
default performance balance_performance balance_power power 

-rw-r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/energy_performance_preference
balance_performance

-r--r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/related_cpus
3

-r--r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/scaling_available_governors
performance powersave

-r--r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq
2500000

-r--r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/scaling_driver
intel_pstate

-rw-r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor
powersave

-rw-r--r-- 1 root root 4.0K Jul  1 10:46 /sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq
2500000

-rw-r--r-- 1 root root 4.0K Jul  1 10:46 /sys/devices/system/cpu/cpu3/cpufreq/scaling_min_freq
2500000

-rw-r--r-- 1 root root 4.0K Jul  1 10:45 /sys/devices/system/cpu/cpu3/cpufreq/scaling_setspeed
<unsupported>

```

`CV = 1.64%`

### v5: cpu frequency 2: scaling governor

```
root@debian-perf-1:/home/ilazaric# echo performance > /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor 
```

```
-rw-r--r-- 1 root root 4.0K Jul  1 12:47 /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor
performance
```

`CV = 1.47%`

### v6: disable aslr

```
ilazaric@debian-perf-1:~$ cat /proc/sys/kernel/randomize_va_space
2
ilazaric@debian-perf-1:~$ su
Password: 
root@debian-perf-1:/home/ilazaric# echo 0 > /proc/sys/kernel/randomize_va_space
root@debian-perf-1:/home/ilazaric# 
exit
ilazaric@debian-perf-1:~$ cat /proc/sys/kernel/randomize_va_space
0
```

`CV = 1.50%`

### v7: `mitigations=off`

also adding `norandmaps`, was achieved already with v6, but this way we don't need to do it on every restart

```
ilazaric@debian-perf-1:~$ cat /proc/sys/kernel/randomize_va_space
0
ilazaric@debian-perf-1:~$ cat /proc/cmdline 
BOOT_IMAGE=/boot/vmlinuz-6.12.94+deb13-amd64 root=UUID=2f5fb668-5b04-4469-9731-f83447e7f283 ro quiet isolcpus=3 irqaffinity=0,1,2 mitigations=off norandmaps
```

`CV = 1.52%`

### v8: no turbo

```
root@debian-perf-1:/home/ilazaric# cat /sys/devices/system/cpu/intel_pstate/no_turbo
0
root@debian-perf-1:/home/ilazaric# echo 1 | tee /sys/devices/system/cpu/intel_pstate/no_turbo
1
root@debian-perf-1:/home/ilazaric# cat /sys/devices/system/cpu/intel_pstate/no_turbo
1
```

`CV = 1.33%`

### v9: perf

added `sysctl.kernel.perf_event_paranoid=-1` kernel param so i can collect stats with perf

```
ilazaric@debian-perf-1:~$ cat /proc/cmdline
BOOT_IMAGE=/boot/vmlinuz-6.12.94+deb13-amd64 root=UUID=2f5fb668-5b04-4469-9731-f83447e7f283 ro quiet isolcpus=3 irqaffinity=0,1,2 mitigations=off norandmaps sysctl.kernel.perf_event_paranoid=-1
```

added binary `delayed_exec` , can talk to `perf` to start measurements right before benchmark exec  
added perfing magic to `run.sh` , and a couple of new columns

cpu-migrations is consistently 0, good  
context-switches though can be really high  
filtering out `context-switches>=10` (~2%) reduces CV quite a bit, from `1.54%` to `0.61%`

### interlude: perf -e context-switches

further testing of `context-switches` event, it is dependent on paranoid value,  
`2` is consistently `0`, but `-1` shows varied results

```
ilazaric@ilazaric-gram:~/repos/ALL/ivl/perf_host/v10$ (for i in {1..100}; do perf stat -e context-switches taskset -c 3 /tmp/benchmark 2>&1 >/dev/null | head -4 | tail -1; done) | sort | uniq -c
      3                  1      context-switches                                                      
      2                  2      context-switches                                                      
      7                  3      context-switches                                                      
     13                  4      context-switches                                                      
     30                  5      context-switches                                                      
     13                  6      context-switches                                                      
     18                  7      context-switches                                                      
      8                  8      context-switches                                                      
      2                  9      context-switches                                                      
      2                 10      context-switches                                                      
      1                 11      context-switches                                                      
      1                 14      context-switches                                                      
```

```
ilazaric@debian-perf-1:~$ (for i in {1..100}; do perf stat -e context-switches taskset -c 3 /tmp/benchmark 2>&1 >/dev/null | head -4 | tail -1; done) | sort | uniq -c
      1                  1      context-switches                                                      
     15                  2      context-switches                                                      
     31                  3      context-switches                                                      
     24                  4      context-switches                                                      
     21                  5      context-switches                                                      
      6                  6      context-switches                                                      
      1                751      context-switches                                                      
      1                911      context-switches                                                      
```

```
ilazaric@ilazaric-gram:~/repos/ALL/ivl/perf_host/v10$ (for i in {1..1000}; do perf stat -e context-switches taskset -c 3 /tmp/benchmark 2>&1 >/dev/null | head -4 | tail -1; done) | sort | uniq -c
      2                  1      context-switches                                                      
     21                  2      context-switches                                                      
     84                  3      context-switches                                                      
    201                  4      context-switches                                                      
    206                  5      context-switches                                                      
    199                  6      context-switches                                                      
    112                  7      context-switches                                                      
     89                  8      context-switches                                                      
     33                  9      context-switches                                                      
     15                 10      context-switches                                                      
     10                 11      context-switches                                                      
      7                 12      context-switches                                                      
      4                 13      context-switches                                                      
      1                 14      context-switches                                                      
      3                 15      context-switches                                                      
      3                 17      context-switches                                                      
      1                 18      context-switches                                                      
      1                 19      context-switches                                                      
      1                 20      context-switches                                                      
      1                 21      context-switches                                                      
      1                 22      context-switches                                                      
      1                 23      context-switches                                                      
      1                 27      context-switches                                                      
      2                 30      context-switches                                                      
      1                 36      context-switches                                                      
```

```
ilazaric@debian-perf-1:~$ (for i in {1..1000}; do perf stat -e context-switches taskset -c 3 /tmp/benchmark 2>&1 >/dev/null | head -4 | tail -1; done) | sort | uniq -c
     11                  1      context-switches                                                      
     97                  2      context-switches                                                      
    315                  3      context-switches                                                      
    335                  4      context-switches                                                      
    161                  5      context-switches                                                      
     55                  6      context-switches                                                      
      6                  7      context-switches                                                      
      1                 32      context-switches                                                      
      1                103      context-switches                                                      
      1                150      context-switches                                                      
      1                212      context-switches                                                      
      1                230      context-switches                                                      
      1                325      context-switches                                                      
      1                459      context-switches                                                      
      1                494      context-switches                                                      
      1                560      context-switches                                                      
      1                606      context-switches                                                      
      1                703      context-switches                                                      
      2                747      context-switches                                                      
      1                748      context-switches                                                      
      2                809      context-switches                                                      
      1                811      context-switches                                                      
      1                839      context-switches                                                      
      1                846      context-switches                                                      
      1                877      context-switches                                                      
```

```
ilazaric@ilazaric-gram:~/repos/ALL/ivl/perf_host$ cat /proc/cmdline 
BOOT_IMAGE=/boot/vmlinuz-6.17.0-35-generic root=UUID=f10b36fd-5baa-4f1b-ad23-b8e4d53e23da ro quiet splash apparmor=0 vt.handoff=7
```

could apparmor be creating noise?

#### kill apparmor

adding `apparmor=0` kernel param

```
root@debian-perf-1:/home/ilazaric# systemctl stop apparmor
root@debian-perf-1:/home/ilazaric# systemctl disable apparmor
Synchronizing state of apparmor.service with SysV service script with /usr/lib/systemd/systemd-sysv-install.
Executing: /usr/lib/systemd/systemd-sysv-install disable apparmor
Removed '/etc/systemd/system/sysinit.target.wants/apparmor.service'.
```

```
ilazaric@debian-perf-1:~$ cat /proc/cmdline 
BOOT_IMAGE=/boot/vmlinuz-6.12.94+deb13-amd64 root=UUID=2f5fb668-5b04-4469-9731-f83447e7f283 ro quiet isolcpus=3 irqaffinity=0,1,2 mitigations=off norandmaps sysctl.kernel.perf_event_paranoid=-1 apparmor=0
```

```
ilazaric@debian-perf-1:~$ (for i in {1..1000}; do perf stat -e context-switches taskset -c 3 /tmp/benchmark 2>&1 >/dev/null | head -4 | tail -1; done) | sort | uniq -c
      3                  1      context-switches                                                      
     99                  2      context-switches                                                      
    321                  3      context-switches                                                      
    313                  4      context-switches                                                      
    193                  5      context-switches                                                      
     46                  6      context-switches                                                      
      4                  7      context-switches                                                      
      4                  8      context-switches                                                      
      1                665      context-switches                                                      
      2                708      context-switches                                                      
      1                727      context-switches                                                      
      1                737      context-switches                                                      
      1                762      context-switches                                                      
      1                772      context-switches                                                      
      1                776      context-switches                                                      
      1                786      context-switches                                                      
      1                793      context-switches                                                      
      1                802      context-switches                                                      
      1                809      context-switches                                                      
      2                810      context-switches                                                      
      1                811      context-switches                                                      
      1                828      context-switches                                                      
      1                847      context-switches                                                      
```

nope

#### nohz_full ?

reading through the series of blogposts: https://www.suse.com/c/cpu-isolation-nohz_full-part-3/  

```
ilazaric@debian-perf-1:~$ cat /proc/cmdline
BOOT_IMAGE=/boot/vmlinuz-6.12.94+deb13-amd64 root=UUID=2f5fb668-5b04-4469-9731-f83447e7f283 ro quiet isolcpus=3 irqaffinity=0,1,2 mitigations=off norandmaps sysctl.kernel.perf_event_paranoid=-1 apparmor=0 nohz_full=3
```

```
ilazaric@debian-perf-1:~$ (for i in {1..1000}; do perf stat -e context-switches taskset -c 3 /tmp/benchmark 2>&1 >/dev/null | head -4 | tail -1; done) | sort | uniq -c
    965                  1      context-switches                                                      
     35                  2      context-switches                                                      
```

incredible!

### v10: nohz_full

from the interlude (mainly suse blog) we concluded `nohz_full=3` kernel param is good to try

i left the apparmor disabling bc im lazy

```
ilazaric@debian-perf-1:~$ cat /proc/cmdline 
BOOT_IMAGE=/boot/vmlinuz-6.12.94+deb13-amd64 root=UUID=2f5fb668-5b04-4469-9731-f83447e7f283 ro quiet isolcpus=3 irqaffinity=0,1,2 mitigations=off norandmaps sysctl.kernel.perf_event_paranoid=-1 apparmor=0 nohz_full=3
```

`CV = 0.64%` , big improvement!

also, max `context-switches` is `2` , the outliers have vanished

### interlude: checking context switches

perf event context-switches combines voluntary and nonvoluntary, but we can separate them via `/proc/$pid/status`

```
ilazaric@ilazaric-gram:~/repos/ALL/ivl/perf_host$ cat /proc/$$/status | grep ctxt
voluntary_ctxt_switches:	17633
nonvoluntary_ctxt_switches:	873
```

problem though, perf reaps the child immediately  
we could hack up perf to dump `/proc/$pid/status` somewhere, but for now we can just get the benchmark to do it  
specifically, dump it before and after (in case taskset incurs some ctxt switches)

```
# generating a bunch of outputs
ilazaric@debian-perf-1:~$ for i in {1..100}; do echo $i ; taskset -c 3 /tmp/bench2 > /tmp/out$i ; done
...
# looking for outputs with diff in ctxt switches
ilazaric@debian-perf-1:~$ for i in {1..100}; do if [ $(cat /tmp/out$i | grep ctxt | sort | uniq | wc -l) -ne 2 ]; then echo $i ; fi; done
8
35
ilazaric@debian-perf-1:~$ cat /tmp/out8 | grep ctxt
voluntary_ctxt_switches:	0
nonvoluntary_ctxt_switches:	1
voluntary_ctxt_switches:	1
nonvoluntary_ctxt_switches:	1
ilazaric@debian-perf-1:~$ cat /tmp/out35 | grep ctxt
voluntary_ctxt_switches:	0
nonvoluntary_ctxt_switches:	1
voluntary_ctxt_switches:	0
nonvoluntary_ctxt_switches:	2
```

so both voluntary and nonvoluntary is possible, kinda weird

i spawned a cpu stress hog , pinned to core 3 , and watched `/proc/$pid/status` , no change in both ctxt switches  
did same with memory hog  
so i guess seen ctxt switches probably related to the few syscalls bench2 calls?

ignoring for now as it seems small, but a TODO to take a look once more

### interlude: data analysis

we added some perf columns, in particular various cache misses, might be good to check if they predict duration well

using data from v10

```
checking perf:cache-misses predictability ...
ratios.std() / ratios.mean() = 8.65%
checking perf:l2_rqsts.all_demand_miss predictability ...
ratios.std() / ratios.mean() = 1.23%
checking perf:L1-dcache-misses predictability ...
ratios.std() / ratios.mean() = 1.17%
```

dunno what that means really

gonna rerun sampling, 10k samples instead of just 1k

saw a big outlier, seems related to cache-misses

