#!/usr/bin/env python3

from pathlib import Path
import shutil

repo_root = Path(__file__).parent
src = repo_root / "src"

build_dir = repo_root / "build"

if build_dir.exists():
    shutil.rmtree(build_dir)
build_dir.mkdir()
include = build_dir / "include" / "ivl"
default_include = build_dir / "default_include" / "ivl"
include.mkdir(parents=True)
default_include.mkdir(parents=True)

hdrs = []
srcs = []

for dirpath, _, filenames in src.walk():
    for filename in filenames:
        filepath = dirpath / filename
        if filepath.suffix == ".cpp":
            srcs.append(filepath)
        if filepath.suffix == ".hpp":
            hdrs.append(filepath)

# print(hdrs)
# print(srcs)

for hdr in hdrs:
    rel = hdr.relative_to(src)
    if hdr.name == "default.hpp":
        (default_include / rel.parent.parent).mkdir(parents=True, exist_ok=True)
        shutil.copyfile(hdr, default_include / rel.parent)
    else:
        (include / rel.parent).mkdir(parents=True, exist_ok=True)
        shutil.copyfile(hdr, include / rel.parent / rel.stem)

# ninjapath = build_dir / "build.ninja"
# ninja = ninjapath.open("w")

# print("""
# rule cp
#   command = cp $in $out

# rule cxx
#   depfile = $out.d
#   command = $cxx -MD -MF $out.d $in -o $out
# """, file=ninja)
