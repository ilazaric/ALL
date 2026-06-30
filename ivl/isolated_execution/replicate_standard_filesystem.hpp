#pragma once

#include <ivl/linux/throwing_syscalls>
#include <filesystem>

namespace ivl::isolated_execution {
// Replicate most of what processes would expect exists on a standard linux,
// for the purposes of pivot_root-ing into the directory.
//
// Relevant: https://refspecs.linuxfoundation.org/FHS_3.0/fhs/index.html
// Noting we doesn't follow FHS identically.
//
// Requires ability to `mount()`, likely from unshared user & mount namespaces.
void replicate_standard_filesystem(const std::filesystem::path& directory) {
  namespace sys = linux::throwing_syscalls;
  auto create_mount = [](const std::filesystem::path& directory, umode_t mode, char* type) {
    sys::mkdir(directory.c_str(), mode);
    sys::mount(type, (char*)directory.c_str(), type, 0, nullptr);
  };
  auto bind_mount = [](const std::filesystem::path& file, const std::filesystem::path& orig, umode_t mode) {
    sys::creat(file.c_str(), mode);
    sys::mount((char*)orig.c_str(), (char*)file.c_str(), "none", MS_BIND, nullptr);
  };
  sys::mount("tmpfs", (char*)directory.c_str(), "tmpfs", 0, nullptr);
  create_mount(directory / "tmp", 0777, "tmpfs");
  // TODO: needs fork
  // create_mount(directory / "proc", 0707, "proc");
  create_mount(directory / "sys", 0707, "sysfs");
  sys::mount("cgroup2", (char*)(directory / "sys/fs/cgroup").c_str(), "cgroup2", 0, nullptr);
  sys::mkdir((directory / "dev").c_str(), 0755);
  bind_mount(directory / "dev/null", "/dev/null", 0770);
  bind_mount(directory / "dev/zero", "/dev/zero", 0770);
  bind_mount(directory / "dev/random", "/dev/random", 0770);
  bind_mount(directory / "dev/urandom", "/dev/urandom", 0770);
  sys::symlink("/proc/self/fd/0", (directory / "dev/stdin").c_str());
  sys::symlink("/proc/self/fd/1", (directory / "dev/stdout").c_str());
  sys::symlink("/proc/self/fd/2", (directory / "dev/stderr").c_str());
}
} // namespace ivl::isolated_execution
