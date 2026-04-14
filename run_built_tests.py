#!/usr/bin/env python3

import glob
from pathlib import Path
import subprocess

repo_root = Path(__file__).parent.resolve()
ivl_dir = repo_root / "ivl"

tests = glob.glob(f"{ivl_dir}/**/*@test", recursive=True)
print(f"[RUNNER] tests: {tests}")

failed = []
for test in tests:
    print(f"[RUNNER] running test {test} ...", flush=True)
    p = subprocess.run([test])
    if p.returncode == 0:
        continue
    print(f"[RUNNER] test {test} failed :(")
    failed.append(test)

if len(failed) == 0:
    exit(0)

print(f"[RUNNER] seen failures: {failed}")
exit(1)
