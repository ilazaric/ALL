#!/usr/bin/env python3

# TODO: move this file into ivl/

from pathlib import Path
import shutil
import sys
import subprocess
import os
from dataclasses import dataclass

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

all_targets = dict()

@dataclass
class TargetState:
    path: Path
    added_compiler_flags: list
    added_compiler_flags_tail: list
    # building this requires these to also be built,
    # but not strictly before
    unordered_dependencies: set

common_test_dependencies = {Path("/build_system/run_test")}
    
def deduce_file_targets(path):
    added_compiler_flags = []
    added_compiler_flags_tail = []
    unordered_dependencies = set()
    unordered_test_dependencies = set()
    file_has_reg_variant = path.suffix == ".cpp"
    file_has_test_variant = False
    def add_compiler_flags(arg):
        nonlocal added_compiler_flags
        added_compiler_flags += arg.split()
    def add_compiler_flags_tail(arg):
        nonlocal added_compiler_flags_tail
        added_compiler_flags_tail += arg.split()
    def add_dependencies(arg):
        nonlocal unordered_dependencies
        unordered_dependencies |= set(arg.split())
    def add_test_dependencies(arg):
        nonlocal unordered_test_dependencies
        unordered_test_dependencies |= set(arg.split())
    def test_only():
        nonlocal file_has_reg_variant
        nonlocal file_has_test_variant
        assert path.suffix == ".cpp", path
        file_has_reg_variant = False
        file_has_test_variant = True
    def has_test_variant():
        nonlocal file_has_test_variant
        file_has_test_variant = True

    with path.open() as f:
        x = "// IVL "
        ivl_directives = [l.removeprefix(x)[:-1] for l in f.readlines() if l.startswith(x)]
    for d in ivl_directives:
        eval(d)

    # TODO: mayhaps don't strip out the ivl , when you add submodules
    if path.name == "default.hpp":
        name = "/" / path.parent.relative_to(src / "ivl")
    else:
        name = "/" / path.relative_to(src / "ivl").with_suffix('')

    if file_has_test_variant:
        all_targets[name.parent / f"{name.name}@test"] = TargetState(path, added_compiler_flags, added_compiler_flags_tail + ["-include", "ivl/reflection/test_runner"], unordered_dependencies | unordered_test_dependencies | common_test_dependencies)
    if file_has_reg_variant:
        all_targets[name] = TargetState(path, added_compiler_flags, added_compiler_flags_tail, unordered_dependencies)

for dirpath, _, filenames in src.walk():
    for filename in filenames:
        filepath = dirpath / filename
        if filepath.suffix == ".cpp" or filepath.suffix == ".hpp":
            deduce_file_targets(filepath)

# print(*all_targets.keys(), sep="\n")

unprocessed_targets = set()
for x in sys.argv[1:]:
    y = repo_root / "ivl" / ("."+x) if x.startswith("/") else Path.cwd() / x
    z = "/" / y.relative_to(repo_root / "ivl")
    assert y.is_dir() or z in all_targets, z
    if z in all_targets:
        unprocessed_targets.add(z)
        continue
    for target in all_targets.keys():
        if z in target.parents:
            unprocessed_targets.add(target)

targets = set()
while unprocessed_targets:
    target = unprocessed_targets.pop()
    targets.add(target)
    for dep in all_targets[target].unordered_dependencies:
        if dep not in targets:
            unprocessed_targets.add(dep)
            
print(targets)

# cxxinc = [f"-I{x}" for x in (build_dir / "include_dirs").iterdir()]
cxxinc = [f"@{build_dir / "include_dirs/args.rsp"}"]

# cxxfmap = [f"-ffile-prefix-map={x}=" for x in [src] + list((build_dir / "include_dirs").iterdir())]
cxxfmap = [f"-ffile-prefix-map={repo_root}/="]

# TODO: add gcc repo as submodule, build it, default to using it
# UPDT: use the reflection repo: https://forge.sourceware.org/marek/gcc.git
cxx = os.getenv("CXX", "g++")
cxxpre = os.getenv("CXXPRE", "")
cxxrpath = os.getenv("CXXRPATH", [f"-Wl,-rpath={Path(cxx).parent.parent / "lib64"}"] if "/" in cxx else [])
cxxver = os.getenv("CXXVER", "26")
cxxpost = os.getenv("CXXPOST", "")

for target in targets:
    path = all_targets[target].path
    cxxadded = all_targets[target].added_compiler_flags
    cxxaddedpost = all_targets[target].added_compiler_flags_tail
    args = ([cxx] +
            cxxpre.split() +
            cxxrpath +
            cxxadded +
            cxxinc +
            cxxfmap +
            ["-DIVL_LOCAL",
             f"-DIVL_FILE=\"{path.relative_to(src)}\"",
             "-static",
             "-O3",
             # "-g1",
             f"-std=c++{cxxver}",
             # f"-I{include.parent.resolve()}",
             # f"-I{default_include.parent.resolve()}",
             "-include",
             "ivl/reflection/test_attribute",
             "-freflection",
             "-include",
             path,
             "-xc++",
             "/dev/null",
             "-o",
             repo_root / "ivl" / target.relative_to('/')] +
            cxxpost.split() + cxxaddedpost)
    print(" ".join([str(x) for x in args]))
    subprocess.run(args, check=True)
    
