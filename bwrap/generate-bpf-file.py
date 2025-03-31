#!/usr/bin/env python3

import seccomp
import sys

with open("good-syscalls.txt", "r") as f:
    ALLOWED_SYSCALLS = f.read().splitlines()

# print(ALLOWED_SYSCALLS)
# exit(0)

f = seccomp.SyscallFilter(defaction=seccomp.KILL_PROCESS)

for name in ALLOWED_SYSCALLS:
    try:
        f.add_rule(seccomp.ALLOW, name)
    except Exception as e:
        print(f"Skipping syscall {name}: {e}", file=sys.stderr)

with open("good-syscalls.bpf", "wb") as bpf_file:
    f.export_bpf(bpf_file)
