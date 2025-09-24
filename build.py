#!/usr/bin/env python3

from pathlib import Path

repo_root = Path(__file__).parent

build_dir = repo_root / "build"
dirs = [repo_root / "src"]

build_dir.mkdir(exist_ok=True)

hdrs = []
srcs = []

for d in dirs:
    for dirpath, _, filenames in d.walk():
        for filename in filenames:
            filepath = dirpath / filename
            if filepath.suffix == ".cpp":
                srcs.append(filepath)
            if filepath.suffix == ".hpp":
                hdrs.append(filepath)

print(hdrs)
print(srcs)

ninjapath = build_dir / "build.ninja"
ninja = ninjapath.open("w")

print("""
rule cp
  command = cp $in $out

rule cxx
  depfile = $out.d
  command = $cxx -MD -MF $out.d $in -o $out
""", file=ninja)
