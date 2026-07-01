# low variance benchmarking

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

