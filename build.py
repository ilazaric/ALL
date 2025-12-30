#!/usr/bin/env python3

# TODO: move this file into ivl/

from pathlib import Path
import shutil
import sys
import subprocess
import os

repo_root = Path(__file__).parent.resolve()
build_dir = repo_root / "build"
src = build_dir / "source_copy"

build_prep = repo_root / "ivl/build_system/generate_build_sources"
assert build_prep.with_suffix(".cpp").exists(), build_prep
if not build_prep.exists():
    print(f"Build prep binary {build_prep} not found, building it ...")
    subprocess.run(["g++", build_prep.with_suffix(".cpp"), "-O3", "-std=c++23", "-o", build_prep], check=True)
if build_prep.with_suffix(".cpp").stat().st_mtime > build_prep.stat().st_mtime:
    print(f"Build prep binary {build_prep} older than sources, rebuilding it ...")
    subprocess.run(["g++", build_prep.with_suffix(".cpp"), "-O3", "-std=c++23", "-o", build_prep], check=True)
subprocess.run([build_prep], check=True)

hdrs = []
srcs = []

for dirpath, _, filenames in src.walk():
    for filename in filenames:
        filepath = dirpath / filename
        if filepath.suffix == ".cpp":
            srcs.append(filepath)
        if filepath.suffix == ".hpp":
            hdrs.append(filepath)

targets = set()
for x in sys.argv[1:]:
    y = repo_root / "ivl" / ("."+x) if x.startswith("/") else Path.cwd() / x
    if not y.is_dir() and y.suffix != ".cpp":
        y = y.with_suffix(".cpp")
    assert y.exists(), f"{x} ({y})"
    y = src / y.relative_to(repo_root)
    if y.is_file():
        targets.add(y)
        continue
    assert y.is_dir(), x
    for root, dirs, files in y.walk():
        for f in files:
            if f.endswith(".cpp"):
                targets.add(root / f)

print(targets)
# cxxinc = [f"-I{x}" for x in (build_dir / "include_dirs").iterdir()]
cxxinc = [f"@{build_dir / "include_dirs/args.rsp"}"]

# cxxfmap = [f"-ffile-prefix-map={x}=" for x in [src] + list((build_dir / "include_dirs").iterdir())]
cxxfmap = [f"-ffile-prefix-map={repo_root}/="]

# TODO: add gcc repo as submodule, build it, default to using it
cxx = os.getenv("CXX", "g++")
cxxpre = os.getenv("CXXPRE", "")
cxxrpath = os.getenv("CXXRPATH", [f"-Wl,-rpath={Path(cxx).parent.parent / "lib64"}"] if "/" in cxx else [])
cxxadded = []
cxxver = os.getenv("CXXVER", "23")
cxxpost = os.getenv("CXXPOST", "")
cxxaddedpost = []

# TODO: scrape includes for these as well, but eventually would want a preprocessor to do that
def add_compiler_flags(arg):
    global cxxadded
    cxxadded += arg.split()
def add_compiler_flags_tail(arg):
    global cxxaddedpost
    cxxaddedpost += arg.split()

for target in targets:
    cxxadded = []
    cxxaddedpost = []
    with target.open() as f:
        x = "// IVL "
        ivl_directives = [l.removeprefix(x)[:-1] for l in f.readlines() if l.startswith(x)]
    # print(ivl_directives)
    for d in ivl_directives:
        eval(d)
    args = ([cxx] +
            cxxpre.split() +
            cxxrpath +
            cxxadded +
            cxxinc +
            cxxfmap +
            ["-DIVL_LOCAL",
             "-O3",
             # "-g1",
             f"-std=c++{cxxver}",
             # f"-I{include.parent.resolve()}",
             # f"-I{default_include.parent.resolve()}",
             target,
             "-o",
             repo_root / target.relative_to(src).with_suffix('')] +
            cxxpost.split() + cxxaddedpost)
    print(" ".join([str(x) for x in args]))
    subprocess.run(args, check=True)
    
