#!/usr/bin/env python3

# TODO: move this file into src/ (or ivl/ eventually)

from pathlib import Path
import shutil
# import argparse
import sys
import subprocess
import os

repo_root = Path(__file__).parent.resolve()
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
        if '#' in filename:
            continue
        filepath = dirpath / filename
        if filepath.suffix == ".cpp":
            srcs.append(filepath)
        if filepath.suffix == ".hpp":
            hdrs.append(filepath)

# print(hdrs)
# print(srcs)

# TODO: default includes dont work: src/foo/default.hpp, src/foo/bar/default.hpp
for hdr in hdrs:
    rel = hdr.relative_to(src)
    if hdr.name == "default.hpp":
        (default_include / rel.parent.parent).mkdir(parents=True, exist_ok=True)
        shutil.copyfile(hdr, default_include / rel.parent)
    else:
        (include / rel.parent).mkdir(parents=True, exist_ok=True)
        shutil.copyfile(hdr, include / rel.parent / rel.stem)

targets = set()
for x in sys.argv[1:]:
    y = src / ("."+x) if x.startswith("/") else Path.cwd() / x
    if not y.is_dir() and y.suffix != ".cpp":
        y = y.with_suffix(".cpp")
    assert y.exists(), x
    if y.is_file():
        targets.add(y)
        continue
    assert y.is_dir(), x
    for root, dirs, files in y.walk():
        for f in files:
            if '#' in f:
                continue
            if f.endswith(".cpp"):
                targets.add(root / f)

# print(repo_root)
print(targets)

cxx = os.getenv("CXX", "g++")
cxxpre = os.getenv("CXXPRE", "")
cxxadded = []
cxxver = os.getenv("CXXVER", "23")
cxxpost = os.getenv("CXXPOST", "")

# TODO: scrape includes for these as well, but eventually would want a preprocessor to do that
def add_compiler_flags(arg):
    global cxxadded
    cxxadded += arg.split()

for target in targets:
    cxxadded = []
    with target.open() as f:
        x = "// IVL "
        ivl_directives = [l.removeprefix(x)[:-1] for l in f.readlines() if l.startswith(x)]
    # print(ivl_directives)
    for d in ivl_directives:
        eval(d)
    args = ([cxx] +
            cxxpre.split() +
            cxxadded +
            ["-DIVL_LOCAL",
             "-O3",
             "-g1",
             f"-std=c++{cxxver}",
             f"-I{include.parent.resolve()}",
             f"-I{default_include.parent.resolve()}",
             target,
             "-o",
             target.parent / target.stem] +
            cxxpost.split())
    print(" ".join([str(x) for x in args]))
    subprocess.run(args, check=True)
    
