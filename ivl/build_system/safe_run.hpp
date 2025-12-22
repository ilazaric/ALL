#pragma once

#include <ivl/linux/file_descriptor>
#include <ivl/linux/terminate_syscalls>
#include <ivl/process>
#include <cassert>
#include <filesystem>
#include <map>
#include <string_view>
#include <vector>

namespace ivl {
namespace sys = ivl::linux::terminate_syscalls;
namespace detail {

  void ro_bind(const std::filesystem::path& in, const std::filesystem::path& out) {
    // std::cerr << in << " -> " << out << std::endl;
    create_directories(out.parent_path());
    if (!is_directory(in)) sys::close(sys::creat(out.c_str(), 0755));
    else sys::mkdir(out.c_str(), 0755);
    sys::mount((char*)in.c_str(), (char*)out.c_str(), nullptr, MS_BIND, nullptr);
    sys::mount(nullptr, (char*)out.c_str(), nullptr, MS_REMOUNT | MS_BIND | MS_RDONLY, nullptr);
  }

  void replicate(const std::filesystem::path& root, std::filesystem::path file) {
    file = absolute(file);
    // assert(!is_directory(file));
    while (is_symlink(file)) {
      ro_bind(file, root / file.relative_path());
      file = file.parent_path() / read_symlink(file);
    }
    // std::cerr << "!!! " << file << std::endl;
    assert(is_regular_file(file) || is_directory(file));
    ro_bind(file, root / file.relative_path());
  }

  // void replicate(const std::filesystem::path& root, const std::vector<std::filesystem::path>& files) {
  //   for (auto&& file : files) replicate(root, file);
  // }

  void full_write_at(int dfd, const char* name, std::string_view data) {
    auto fd = sys::openat(dfd, name, O_WRONLY, 0);
    while (!data.empty()) data.remove_prefix(sys::write(fd, data.data(), data.size()));
    sys::close(fd);
  }

  void full_write(int fd, std::string_view data) {
    while (!data.empty()) data.remove_prefix(sys::write(fd, data.data(), data.size()));
  }
} // namespace detail

// Files don't have to exist.
// If an output file doesn't exist, the mmap_region is empty.
std::map<std::filesystem::path, ivl::linux::owned_file_descriptor> safe_run(
  process_config pc, const std::vector<std::filesystem::path>& inputs,
  const std::vector<std::filesystem::path>& outputs, const std::filesystem::path& wd, size_t max_memory,
  size_t max_cpu_percentage
) {
  // Setting up the cgroup.
  std::string child_cgroup_name = "child.XXXXXX.slice";
  auto parent_cgroup_fd = sys::open(
    "/sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/ivl-isolation.slice", O_RDONLY | O_DIRECTORY,
    0
  );
  while (true) {
    for (int i = 0; i < 6; ++i) child_cgroup_name[6 + i] = '0' + rand() % 10;
    auto fd = ivl::linux::raw_syscalls::openat(parent_cgroup_fd, child_cgroup_name.c_str(), O_RDONLY | O_DIRECTORY, 0);
    if (fd < 0) break;
    sys::close(fd);
  }
  sys::mkdirat(parent_cgroup_fd, child_cgroup_name.c_str(), 0777);
  auto cgroup_fd = sys::openat(parent_cgroup_fd, child_cgroup_name.c_str(), O_RDONLY | O_DIRECTORY, 0);
  detail::full_write_at(cgroup_fd, "memory.swap.max", "0");
  detail::full_write_at(cgroup_fd, "memory.zswap.max", "0");
  detail::full_write_at(cgroup_fd, "memory.max", std::to_string(max_memory));
  detail::full_write_at(cgroup_fd, "cpu.max", std::format("{}000 100000", max_cpu_percentage));
  pc.cgroup = cgroup_fd;
  std::filesystem::path root = "/dev/shm/mount-point";

  pc.share_fds = true;
  pc.isolate_all = true;
  int tmpfsfd = 0;
  // sys::unlinkat(parent_cgroup_fd, child_cgroup_name.c_str(), AT_REMOVEDIR);

  pc.pre_exec([&] {
    {
      int fd;

      fd = ivl::linux::terminate_syscalls::openat(AT_FDCWD, "/proc/self/uid_map", O_WRONLY, 0);
      detail::full_write(fd, "0 1000 1");
      ivl::linux::terminate_syscalls::close(fd);

      fd = ivl::linux::terminate_syscalls::openat(AT_FDCWD, "/proc/self/setgroups", O_WRONLY, 0);
      detail::full_write(fd, "deny");
      ivl::linux::terminate_syscalls::close(fd);

      fd = ivl::linux::terminate_syscalls::openat(AT_FDCWD, "/proc/self/gid_map", O_WRONLY, 0);
      detail::full_write(fd, "0 1000 1");
      ivl::linux::terminate_syscalls::close(fd);
    }

    sys::mount("tmpfs", (char*)root.c_str(), "tmpfs", 0, /*size={} could go here*/ nullptr);
    tmpfsfd = sys::open(root.c_str(), O_RDONLY, 0);
    for (auto&& input : inputs) detail::replicate(root, wd / input);
    sys::chroot(root.c_str());
    sys::chdir(wd.c_str());
  });

  {
    auto ret = pc.clone_and_exec().unwrap_or_terminate().wait();
    assert(ret.unwrap_or_terminate() == 0);
    assert(tmpfsfd > 0);
  }

  sys::unlinkat(parent_cgroup_fd, child_cgroup_name.c_str(), AT_REMOVEDIR);
  sys::close(parent_cgroup_fd);
  sys::close(cgroup_fd);

  std::map<std::filesystem::path, ivl::linux::owned_file_descriptor> ret;
  for (auto&& file : outputs) {
    auto fd = ivl::linux::raw_syscalls::openat(tmpfsfd, (wd / file).relative_path().c_str(), O_RDONLY, 0);
    assert(fd >= 0 || fd == -ENOENT);
    ret[file] = fd >= 0 ? ivl::linux::owned_file_descriptor{(int)fd} : ivl::linux::owned_file_descriptor{};
  }

  sys::close(tmpfsfd);
  return ret;
}
} // namespace ivl
