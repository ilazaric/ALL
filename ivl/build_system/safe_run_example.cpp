#include <ivl/build_system/safe_run>

// IVL add_compiler_flags("-static -flto -Wno-write-strings")

namespace sys = ivl::linux::terminate_syscalls;

int main() {
  std::vector<std::filesystem::path> inputs{
    // who knows, strace showed these
    "/lib/x86_64-linux-gnu/libbfd-2.42-system.so",
    "/lib/x86_64-linux-gnu/libc.so.6",
    "/lib/x86_64-linux-gnu/libctf.so.0",
    "/lib/x86_64-linux-gnu/libgmp.so.10",
    "/lib/x86_64-linux-gnu/libisl.so.23",
    "/lib/x86_64-linux-gnu/libjansson.so.4",
    "/lib/x86_64-linux-gnu/libmpc.so.3",
    "/lib/x86_64-linux-gnu/libmpfr.so.6",
    "/lib/x86_64-linux-gnu/libm.so.6",
    "/lib/x86_64-linux-gnu/libsframe.so.1",
    "/lib/x86_64-linux-gnu/libz.so.1",
    "/lib/x86_64-linux-gnu/libzstd.so.1",
    "tiny.cpp",
    "/usr/bin/as",
    "/usr/bin/g++",
    "/usr/bin/ld",
    "/usr/libexec/gcc/x86_64-linux-gnu/13/cc1plus",
    "/usr/libexec/gcc/x86_64-linux-gnu/13/collect2",
    "/usr/libexec/gcc/x86_64-linux-gnu/13/liblto_plugin.so",
    "/usr/libexec/gcc/x86_64-linux-gnu/13/lto1",
    "/usr/libexec/gcc/x86_64-linux-gnu/13/lto-wrapper",
    "/usr/lib/gcc/x86_64-linux-gnu/13/crtbeginT.o",
    "/usr/lib/gcc/x86_64-linux-gnu/13/crtend.o",
    "/usr/lib/gcc/x86_64-linux-gnu/13/libgcc.a",
    "/usr/lib/gcc/x86_64-linux-gnu/13/libgcc_eh.a",
    "/usr/lib/gcc/x86_64-linux-gnu/13/libstdc++.a",
    "/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/crt1.o",
    "/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/crti.o",
    "/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/crtn.o",
    "/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/libc.a",
    "/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/libm.a",
    "/usr/lib/x86_64-linux-gnu/libm-2.39.a",
    "/usr/lib/x86_64-linux-gnu/libmvec.a",
    // dynamic linker
    "/lib64/ld-linux-x86-64.so.2",
    // standard headers
    "/usr/include",
    "/usr/local/include",
    "/usr/lib/gcc/x86_64-linux-gnu/13/include",

    "/usr/bin/ls",
    // "/proc",
    "/lib/x86_64-linux-gnu/libselinux.so.1",
    "/lib/x86_64-linux-gnu/libpcre2-8.so.0",
    "/lib/x86_64-linux-gnu/libtinfo.so.6",
    "/usr/bin/bash",
    "/usr/bin/file",
    "/lib/x86_64-linux-gnu/libmagic.so.1",
    "/lib/x86_64-linux-gnu/liblzma.so.5",
    "/lib/x86_64-linux-gnu/libbz2.so.1.0",
    "/usr/share/misc/magic.mgc",
    "/usr/bin/cat",
    // "foo",
    "/usr/bin/id",
    "/lib/x86_64-linux-gnu/libunwind-ptrace.so.0",
    "/lib/x86_64-linux-gnu/libunwind-x86_64.so.8",
    "/lib/x86_64-linux-gnu/libunwind.so.8",
    "/usr/bin/strace",
  };

  std::vector<std::filesystem::path> outputs{
    "a.out",
  };

  ivl::process_config pc;
  pc.pathname = "/usr/bin/bash";
  pc.argv = {"/usr/bin/bash"};
  // pc.pathname = "/usr/bin/g++";
  // pc.argv = {"/usr/bin/g++", "-static", "-frandom-seed=42", "-flto", "tiny.cpp"};
  pc.envp = {{"PATH", "/usr/bin"}, {"LC_ALL", "C"}};

  auto ret = ivl::safe_run(std::move(pc), inputs, outputs, std::filesystem::current_path(), 200ULL << 20, 100);
  LOG(ret.wstatus);
  for (auto&& [p, fd] : ret.outputs) LOG(p, fd.get());
  // while (true);

  auto&& fd = ret.outputs["a.out"];
  assert(!fd.empty());
  sys::execveat(fd.get(), "", nullptr, nullptr, AT_EMPTY_PATH);
}
