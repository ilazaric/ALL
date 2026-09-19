#!/usr/bin/env python3

# TODO: move this file into ivl/

from pathlib import Path
import shutil
import sys
import subprocess
import os
from dataclasses import dataclass
import time
import argparse
import multiprocessing

def check_positive(value):
    ivalue = int(value)
    if ivalue <= 0:
        raise argparse.ArgumentTypeError("%s is an invalid positive int value" % value)
    return ivalue

parser = argparse.ArgumentParser()
parser.add_argument('-v', '--verbose', action='store_true')
parser.add_argument('-j', '--jobs', '--parallel', default=1, type=check_positive)
parser.add_argument('-k', '--keep-going', action='store_true')
parser.add_argument('--syntax-only', action='store_true')
parser.add_argument('--report-durations', action='store_true')
parser.add_argument('-O', '--optimization', default='3', choices=['0', '1', '2', '3', 'g', 's', 'z'])
parser.add_argument('-g', '--debug-info', default='1', choices=['0', '1', '2', '3'])
parser.add_argument('--static', action='store_true')
parser.add_argument('--cxx', default='g++')
parser.add_argument('--cxx-pre', default='')
parser.add_argument('--cxx-rpath')
parser.add_argument('--cxx-version', default='29')
parser.add_argument('--cxx-post', default='')
parser.add_argument('targets', nargs='*')
args = parser.parse_args()
if args.cxx_rpath is None:
    args.cxx_rpath = f"-Wl,-rpath={Path(shutil.which(args.cxx)).parent.parent / 'lib64'}"
# print(args)
# exit(1)

repo_root = Path(__file__).parent.resolve()
build_dir = repo_root / "build"
src = build_dir / "source_copy"
regsrc = build_dir / "include_dirs" / "regular"

build_prep = repo_root / "ivl/build_system/generate_build_sources"
assert build_prep.with_suffix(".cpp").exists(), build_prep
def build_build_prep():
    subprocess.run(["g++", build_prep.with_suffix(".cpp"), "-O3", "-std=c++23", "-static", "-o", build_prep], check=True)
if not build_prep.exists():
    print(f"Build prep binary {build_prep} not found, building it ...")
    build_build_prep()
if build_prep.with_suffix(".cpp").stat().st_mtime > build_prep.stat().st_mtime:
    print(f"Build prep binary {build_prep} older than sources, rebuilding it ...")
    build_build_prep()
subprocess.run([build_prep], check=True)

if not args.syntax_only:
    modsrc = build_dir / "submodule_source_copy"
    modobj = build_dir / "submodule_objdir"
    modobj.mkdir(exist_ok=True)
    def cmake_submodule(m, target):
        S = modsrc / m
        B = modobj / m
        if not (B / "build.ninja").exists():
            subprocess.run(["cmake", "-DCMAKE_CXX_STANDARD=26", "-DCMAKE_BUILD_TYPE=RelWithDebInfo", "-S", S, "-B", B, "-G", "Ninja"], check=True)
        subprocess.run(["cmake", "--build", B, "--target", target, "--parallel", f"{args.jobs}"], check=True)
    cmake_submodule("fmt", "libfmt.a")
    cmake_submodule("pugixml", "libpugixml.a")
    # cmake_submodule("nlohmann-json") # no libraries
    cmake_submodule("raylib", "libraylib.a")
    libs = build_dir / "libraries"
    libs.mkdir(exist_ok=True)
    shutil.copy(modobj / "raylib/raylib/libraylib.a", libs / "libraylib.a")
    shutil.copy(modobj / "pugixml/libpugixml.a", libs / "libpugixml.a")
    shutil.copy(modobj / "fmt/libfmt.a", libs / "libfmt.a")
    # TODO: clean up
    libs_link = [f"-L{libs}", "-lfmt", "-lpugixml", "-lraylib"] + "-lm  -lpthread  -lGLU  -lm  -lrt  -lm  -ldl".split()
else:
    libs_link = []

all_targets = dict()

@dataclass
class TargetState:
    path: Path
    added_compiler_flags: list
    added_compiler_flags_tail: list
    # building this requires these to also be built,
    # but not strictly before
    unordered_dependencies: set

# TODO: revert when build is caching
common_test_dependencies = set() # {Path("/build_system/run_test")}
    
def deduce_file_targets(path):
    added_compiler_flags = []
    added_compiler_flags_tail = []
    unordered_dependencies = set()
    unordered_test_dependencies = set()
    file_has_reg_variant = path.suffix == ".cpp"
    file_has_test_variant = False
    ivl_main_handler = True
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
    def disable_ivl_main_handler():
        nonlocal ivl_main_handler
        ivl_main_handler = False

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
        all_targets[name.parent / f"{name.name}@test"] = TargetState(path, added_compiler_flags, libs_link + added_compiler_flags_tail + ["-include", "ivl/reflection/test_runner"], unordered_dependencies | unordered_test_dependencies | common_test_dependencies)
    if file_has_reg_variant:
        all_targets[name] = TargetState(path, added_compiler_flags, libs_link + added_compiler_flags_tail + (["-include", "ivl/reflection/ivl_main_handler"] if ivl_main_handler else []), unordered_dependencies)
    all_targets[name.parent / f"{name.name}@syntax_only"] = TargetState(path, ["-fsyntax-only"] + added_compiler_flags, added_compiler_flags_tail, unordered_dependencies)

for dirpath, _, filenames in src.walk():
    for filename in filenames:
        filepath = dirpath / filename
        if filepath.suffix == ".cpp" or filepath.suffix == ".hpp":
            deduce_file_targets(filepath)

unprocessed_targets = set()
for x in args.targets:
    y = repo_root / "ivl" / ("."+x) if x.startswith("/") else Path.cwd() / x
    y = y.resolve() # TODO: does it do good with symlinks?
    assert repo_root / "ivl" in y.parents or repo_root / "ivl" == y, x
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

if args.syntax_only:
    targets = [t for t in targets if str(t).endswith("@syntax_only")]
    targets = [t for t in targets if not str(t).startswith("/edg-reflection")]
    targets = [t for t in targets if not str(t).startswith("/cpp-parser")]
    targets = [t for t in targets if not str(t).startswith("/alloc")]
    targets = [t for t in targets if not str(t).endswith("_X@syntax_only")]
    targets = [t for t in targets if str(t).endswith("@syntax_only")]
else:
    targets = [t for t in targets if not str(t).endswith("@syntax_only")]
for t in targets:
    print(all_targets[t].path.relative_to(src))
print(f"{len(targets) = }")

cxxinc = [f"@{build_dir / "include_dirs/args.rsp"}"]
cxxfmap = [f"-ffile-prefix-map={repo_root}/="]

# TODO: add gcc repo as submodule, build it, default to using it
# UPDT: use the reflection repo: https://forge.sourceware.org/marek/gcc.git
# UPDT: reflection merged upstream, also submodules/build-gcc.sh installs it to /opt/GCC
cxx = args.cxx
cxxpre = args.cxx_pre
cxxrpath = args.cxx_rpath
cxxver = args.cxx_version
cxxpost = args.cxx_post

def run_target(target):
    path = all_targets[target].path
    relpath = path.relative_to(src)
    incpath = None
    if relpath.name == "default.hpp":
        incpath = relpath.parent
    elif relpath.suffix == ".hpp":
        incpath = relpath.parent / relpath.stem
    elif relpath.suffix == ".cpp":
        incpath = relpath
    else:
        assert False, relpath
    assert incpath, relpath
    if incpath.suffix == ".cpp":
        # otherwise running from root would find in-git file, not copy
        incpath = regsrc / incpath

    cxxadded = all_targets[target].added_compiler_flags
    cxxaddedpost = all_targets[target].added_compiler_flags_tail
    cmd = ([cxx] +
           cxxpre.split() +
           [cxxrpath] +
           cxxadded +
           cxxinc +
           cxxfmap +
           ["-DIVL_LOCAL", f"-DIVL_FILE=\"{relpath}\""] +
           (["-static"] if args.static else []) +
           [f"-O{args.optimization}",
            f"-g{args.debug_info}",
            f"-std=c++{cxxver}",
            "-freflection",
            "-fcontracts",
            "-include",
            incpath,
            "-xc++",
            "/dev/null",
            "-o",
            repo_root / "ivl" / target.relative_to('/')] +
           cxxpost.split() + cxxaddedpost)
    if args.verbose: print(" ".join([str(x) for x in cmd]))
    start = time.perf_counter()
    p = subprocess.run(cmd, check=not args.keep_going)
    elapsed = time.perf_counter() - start
    return (target, p, elapsed)

failed = []
durations = dict()
total_seen = 0
pool = multiprocessing.Pool(processes=args.jobs)
for target, p, elapsed in pool.imap_unordered(run_target, targets):
    durations[target] = elapsed
    total_seen += 1
    if p.returncode != 0:
        failed.append(target)

if args.report_durations:
    print()
    ordered = sorted(targets, key = lambda target: -durations[target])
    limit = 20
    print(f"Slowest {limit} files:")
    for t in ordered[:20]:
        print(f"  {durations[t]:.2f}s -- {all_targets[t].path.relative_to(src)}")
    print()
    accumulated = dict()
    children = dict()
    for t in targets:
        p = all_targets[t].path
        while True:
            accumulated[p] = 0.0
            children[p] = set()
            if p == src: break
            p = p.parent
    for t in targets:
        p = all_targets[t].path
        while True:
            accumulated[p] += durations[t]
            if p == src: break
            children[p.parent].add(p)
            p = p.parent
    def dumpit(p, cs):
        prefix = ""
        for c in cs[:-1]:
            prefix += "|   " if c else "    "
        if cs:
            prefix += "|-- " if cs[-1] else "`-- "
        rel = p.relative_to(src)
        print(prefix + (str(rel.name) if p != src else "<ROOT>"), f"-- {accumulated[p]:.2f}s")
        nxt = sorted(list(children[p]), key = lambda x: -accumulated[x])
        for child in nxt[:-1]:
            dumpit(child, cs + [True])
        if nxt:
            dumpit(nxt[-1], cs + [False])
    print(f"Durations by filesystem tree structure:")
    dumpit(src, [])


if not failed:
    print()
    print("all targets passed")
    assert total_seen == len(targets)
    exit(0)

print()
print("seen failures:", len(failed))
for target in failed:
    print(" ", target)
assert total_seen == len(targets)
exit(1)

