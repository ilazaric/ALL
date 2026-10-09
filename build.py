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
import re

def check_positive(value):
    ivalue = int(value)
    if ivalue <= 0:
        raise argparse.ArgumentTypeError("%s is an invalid positive int value" % value)
    return ivalue

# chatgpt generated thing, to print cmdline arg defaults
# argparse.ArgumentDefaultsHelpFormatter dont work if no help= set
class DefaultsHelpFormatter(argparse.ArgumentDefaultsHelpFormatter):
    def _format_action(self, action):
        if action.help is None and action.default is not argparse.SUPPRESS and action.default is not None:
            action.help = "(default: %(default)s)"
            try:
                return super()._format_action(action)
            finally:
                action.help = None
        return super()._format_action(action)

parser = argparse.ArgumentParser(
    formatter_class=DefaultsHelpFormatter,
)
parser.add_argument('--skip-known-failures', action=argparse.BooleanOptionalAction, default=True)
parser.add_argument('-v', '--verbose', action='store_true')
parser.add_argument('-j', '--jobs', '--parallel', default=1, type=check_positive)
parser.add_argument('-k', '--keep-going', action='store_true')
parser.add_argument('--syntax-only', action='store_true')
parser.add_argument('--report-durations', action='store_true')
parser.add_argument('-O', '--optimization', default='3', choices=['0', '1', '2', '3', 'g', 's', 'z'])
parser.add_argument('-g', '--debug-info', default='1', choices=['0', '1', '2', '3'])
parser.add_argument('--static', action='store_true')
parser.add_argument('--cxx', default='g++')
parser.add_argument('--cxx-pre')
parser.add_argument('--cxx-rpath', help="default: {cxx}/../../lib64")
parser.add_argument('--cxx-version', help="defaults: gcc==29, edg==26")
parser.add_argument('--cxx-post')
parser.add_argument('--with-system-libstdcxx')
parser.add_argument('--with-custom-libstdcxx')
parser.add_argument('--edg', action='store_true')
parser.add_argument('targets', nargs='*')
args = parser.parse_args()
if args.cxx_version is None:
    args.cxx_version = "26" if args.edg else "29"
if args.cxx_rpath is None:
    args.cxx_rpath = f"-Wl,-rpath={Path(shutil.which(args.cxx)).parent.parent / 'lib64'}"
# instead of default='' doing this, --help is nicer that way imo
if args.cxx_post is None:
    args.cxx_post = ""
if args.cxx_pre is None:
    args.cxx_pre = ""
assert args.with_system_libstdcxx is None or args.with_custom_libstdcxx is None
assert not args.edg or not args.with_custom_libstdcxx
assert not args.edg or not args.with_system_libstdcxx
assert not args.edg or not args.static
assert not args.edg or args.cxx == 'g++'

repo_root = Path(__file__).parent.resolve()
build_dir = repo_root / "build"
src = build_dir / "source_copy"
regsrc = build_dir / "include_dirs" / "regular"
empty = build_dir / "empty.cpp"
edg_cfg = build_dir / "edg_gcc"
build_dir.mkdir(exist_ok=True)
with empty.open("w", encoding="utf-8") as f:
    f.write("")

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

if args.edg:
    # TODO: copy submodule first
    subprocess.run([repo_root / "submodules/build-edg.sh"], check=True)

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
    libs_link = [f"-L{libs}", "-lfmt", "-lpugixml", "-lraylib", "-lboost_json"] + "-lm  -lpthread  -lGLU  -lm  -lrt  -lm  -ldl".split()
    if args.edg:
        # for system installed boost_json
        libs_link = ["-L/usr/lib/x86_64-linux-gnu"] + libs_link
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

cmdline_include = "-include" if not args.edg else "--preinclude"
fsyntax_only = "-fsyntax-only" if not args.edg else "--prelink_objects"
    
def deduce_file_targets(path):
    # TODO: should be --sys_include for EDG?
    added_compiler_flags = [] if args.with_system_libstdcxx is None else [
        "-nostdinc++",
        f"-I/usr/include/c++/{args.with_system_libstdcxx}",
        f"-I/usr/include/x86_64-linux-gnu/c++/{args.with_system_libstdcxx}",
        f"-I/usr/include/c++/{args.with_system_libstdcxx}/backward",
    ]
    added_compiler_flags += [] if args.with_custom_libstdcxx is None else [
        "-nostdinc++",
        f"-I{args.with_custom_libstdcxx}",
        f"-I{args.with_custom_libstdcxx}/x86_64-pc-linux-gnu",
        f"-I{args.with_custom_libstdcxx}/backward",
    ]
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
        all_targets[name.parent / f"{name.name}@test"] = TargetState(path, ["-DIVL_KIND_TEST"] + added_compiler_flags, libs_link + added_compiler_flags_tail + [cmdline_include, "ivl/reflection/test_runner"], unordered_dependencies | unordered_test_dependencies | common_test_dependencies)
    if file_has_reg_variant:
        all_targets[name] = TargetState(path, added_compiler_flags, libs_link + added_compiler_flags_tail + ([cmdline_include, "ivl/reflection/ivl_main_handler"] if ivl_main_handler else []), unordered_dependencies)
    all_targets[name.parent / f"{name.name}@syntax_only"] = TargetState(path, [fsyntax_only] + added_compiler_flags, added_compiler_flags_tail, unordered_dependencies)

ignored_files = set()
for dirpath, _, filenames in src.walk():
    ivlbuild = dirpath / ".ivlbuild"
    if ivlbuild.exists():
        with ivlbuild.open() as f:
            for l in f:
                l = l.strip()
                if l == "": continue
                assert l.startswith("ignore "), l
                ignored_files.add(dirpath / l[7:])
    for filename in filenames:
        filepath = dirpath / filename
        if filepath in ignored_files: continue
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
targets = sorted(list(targets))

known_failures = [
    "/edg-reflection/", # these were deployed to godbolt, edg compiler w reflection
    "/cpp-parser/",
    "/alloc/",
    "/langs/preprocessor/",
    "/experiments/implcit_args/bug2",
    "/reflection/fmt_manual_format_test",

    "/build_system/manifest",
    "/build_system/throttled_task_executor",
    "/cf/1909/H2",
    "/json/nlohmann_example",
    "/parsing/cmake",
    "/structs/binary_tree",
    "/structs/binary_tree_example",
]

ignored_targets = []
if args.skip_known_failures:
    for regex in known_failures:
        pattern = regex if regex.endswith("/") else regex + "@"
        dropped = [t for t in targets if str(t) == regex or str(t).startswith(pattern)]
        ignored_targets += dropped
        targets = [t for t in targets if t not in dropped]
ignored_targets = sorted(ignored_targets)

if args.syntax_only:
    targets = [t for t in targets if str(t).endswith("@syntax_only")]
    targets = [t for t in targets if not str(t).endswith("_X@syntax_only")]
else:
    targets = [t for t in targets if not str(t).endswith("@syntax_only")]

def print_table(ll):
    widths = []
    for l in ll:
        ws = [len(str(el)) for el in l]
        while len(widths) < len(ws): widths.append(0)
        for i in range(len(ws)): widths[i] = max(widths[i], ws[i])
    for l in ll:
        print(" ".join(f"{str(x):<{w}}" for x, w in zip(l, widths)))

print()
print(f"Building {len(targets)} targets:")
print_table([[t, "--", all_targets[t].path.relative_to(src)] for t in targets])
print()
if ignored_targets: print(f"Ignored {len(ignored_targets)} targets:")
print_table([[t, "--", all_targets[t].path.relative_to(src)] for t in ignored_targets])
if ignored_targets: print()

print(flush=True)

# argsrsp = build_dir / "include_dirs/args.rsp"
# print(sum([["-I", l[4:-1]] for l in argsrsp.read_text().split('\n')[:-1]], []))
# exit(0)

argsrsp = build_dir / "include_dirs/args.rsp"
cxxinc = []
isystem = "-isystem" if not args.edg else "--sys_include"
for line in argsrsp.read_text().strip().split('\n'):
    if line.startswith("-I "): cxxinc += ["-I", line[4:-1]]
    elif line.startswith("-isystem "): cxxinc += [isystem, line[10:-1]]
    else: assert False, line

cxxfmap = [f"-ffile-prefix-map={repo_root}/="] if not args.edg else []

# TODO: add gcc repo as submodule, build it, default to using it
# UPDT: use the reflection repo: https://forge.sourceware.org/marek/gcc.git
# UPDT: reflection merged upstream, also submodules/build-gcc.sh installs it to /opt/GCC
cxx = args.cxx if not args.edg else str(repo_root / "submodules/objdir/edg/bin/eccp")
cxxpre = args.cxx_pre
cxxrpath = [args.cxx_rpath] if not args.edg else ["--c_to_obj_option", args.cxx_rpath]
cxxver = args.cxx_version
cxxpost = args.cxx_post
# injection is for ivl_main_handler edg-specific main synthesis
cxxrefl = ["-freflection"] if not args.edg else ["--set_flag", "reflection", "--set_flag", "injection"]
cxxcontr = ["-fcontracts"] if not args.edg else ["-Dcontract_assert(...)=assert(__VA_ARGS__)"]
cxxstd = f"-std=c++{cxxver}" if not args.edg else f"--c++{cxxver}"
cxxkind = "-xc++" if not args.edg else "--c++"
# cmdline_parsing::implicit
cxxwarn = ["-Wsfinae-incomplete=0"] if not args.edg else []

def foo(a):
    return subprocess.run([f"{repo_root}/submodules/edg-compiler/dev_tools/bin/edg-scrape-compiler", "gcc"] + a, check=True, capture_output=True).stdout.decode("utf-8").strip()

env = os.environ.copy()
env["LC_ALL"] = "C"
if args.edg:
    # env["EDG_BASE"] = str(edg_cfg)
    # env["PATH"] = "/bin:/usr/bin"
    env["EDG_USE_SYSTEM_HEADERS"] = "1"
    # str(repo_root / "submodules/edg-compiler/bases/docker/dev-env/gcc/include") + ":" + 
    env["EDG_GCC_INCL_SCRAPE"] = foo(["--lang", "c++", "includes"])
    # str(repo_root / "submodules/edg-compiler/bases/docker/dev-env/gcc/include_c99") + ":" + 
    env["EDG_GCC_CINCL_SCRAPE"] = foo(["--lang", "c", "includes"])
    env["EDG_GCC_VER_SCRAPE"] = foo(["version"])

# print(env)

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
           cxxrpath +
           cxxadded +
           cxxinc +
           cxxfmap +
           cxxrefl +
           cxxcontr +
           [cxxstd] +
           [cxxkind] +
           cxxwarn +
           ["-DIVL_LOCAL", f"-DIVL_FILE=\"{relpath}\""] +
           (["-static"] if args.static else []) +
           [f"-O{args.optimization}",
            f"-g{args.debug_info}",
            cmdline_include,
            incpath,
            "-o",
            repo_root / "ivl" / target.relative_to('/')] +
           [f"{empty}"] +
           cxxpost.split() + cxxaddedpost
           )
    if args.verbose: print(" ".join([str(x) for x in cmd]))
    start = time.perf_counter()
    p = subprocess.run(cmd, env=env, check=not args.keep_going)
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

print(flush=True)

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
    accumulated[src] = 0.0
    children[src] = []
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

