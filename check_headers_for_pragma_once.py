#!/usr/bin/env python3

from pathlib import Path
from more_itertools import partition

repo_root = Path(__file__).parent.resolve()
srcdir = repo_root / "ivl"

headers = [entry for entry in srcdir.iterdir() if entry.is_file() or entry.suffix == ".hpp"]
headers = list(srcdir.rglob("*.hpp"))

print(f"{len(headers) = }")
print()

print("discovered headers:")
for header in headers:
    print("|", header.relative_to(repo_root))
print("discovered headers end")
print()

print("filtering out files with '#' , emacs likes to create them ...")
headers, ignored = partition(lambda p: "#" in p.name, headers)
headers = list(headers)
ignored = list(ignored)

print("|", f"{len(headers) = }")
print("|", f"{len(ignored) = }")
print("|")

print("| ignored headers:")
for header in ignored:
    print("| |", header.relative_to(repo_root))
print("| ignored headers end")
print("done filtering out '#' files")
print()

print("filtering out _X headers, as they are designed to be included multiple times ...")
headers, ignored = partition(lambda p: p.stem.endswith("_X"), headers)
headers = list(headers)
ignored = list(ignored)

print("|", f"{len(headers) = }")
print("|", f"{len(ignored) = }")
print("|")

print("| ignored headers:")
for header in ignored:
    print("| |", header.relative_to(repo_root))
print("| ignored headers end")
print("done filtering out _X headers")
print()

print("checking if headers start with pragma once ...")

success = []
failed = []

for header in headers:
    line = ""
    with header.open() as f:
        line = f.readline()
    if line == "#pragma once\n":
        success.append(header)
    else:
        failed.append((header, line))

print("|", f"{len(success) = }")
print("|", f"{len(failed) = }")
print("|")
print("| failures and their first lines:")
for header, line in failed:
    print("| |", header.relative_to(repo_root), repr(line))
print("| ignored headers end")

print("done checking if headers start with pragma once")
print()

if len(failed) == 0:
    print("checking fully passed")
    exit(0)
else:
    print("some headers failed checking, exiting non-zero")
    exit(1)

