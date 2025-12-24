#pragma once

#include <ivl/linux/file_descriptor>
#include <ivl/linux/terminate_syscalls>
#include <ivl/process>
#include <cassert>
#include <filesystem>
#include <linux/capability.h>
#include <map>
#include <string_view>
#include <vector>

namespace ivl {
namespace sys = ivl::linux::terminate_syscalls;
namespace detail {
  void ro_bind(const std::filesystem::path& in, const std::filesystem::path& out) {
    if (in.native().contains("g++")) LOG(in, out);
    create_directories(out.parent_path());
    if (!is_directory(in)) sys::close(sys::creat(out.c_str(), 0555));
    else sys::mkdir(out.c_str(), 0555);
    // TODO: symlinks look like files, fix it
    // UPDT: mightve fixed it
    // TODO: permissions don't look correct:
    // ....: $ ls -lah /usr/bin/g++
    // ....: -rwxr-xr-x 1 65534 65534 1004K Sep  4  2024 /usr/bin/g++
    // ....:   ^ bad
    // UPDT: actually this is kinda correct, the wrong thing is we are root

    // This seems to mimick symlinks better.
    auto srcfd = sys::open_tree(AT_FDCWD, in.c_str(), AT_NO_AUTOMOUNT | AT_SYMLINK_NOFOLLOW | OPEN_TREE_CLONE);
    mount_attr attr{};
    attr.attr_set = MOUNT_ATTR_RDONLY;
    sys::mount_setattr(srcfd, "", AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT, &attr, sizeof(attr));
    sys::move_mount(srcfd, "", AT_FDCWD, out.c_str(), MOVE_MOUNT_F_EMPTY_PATH);
    sys::close(srcfd);

    // sys::mount((char*)in.c_str(), (char*)out.c_str(), nullptr, MS_BIND | MS_NOSYMFOLLOW, nullptr);
    // sys::mount(nullptr, (char*)out.c_str(), nullptr, MS_REMOUNT | MS_BIND | MS_RDONLY | MS_NOSYMFOLLOW, nullptr);
  }

  void replicate(const std::filesystem::path& root, std::filesystem::path file) {
    file = absolute(file);
    while (is_symlink(file)) {
      ro_bind(file, root / file.relative_path());
      file = file.parent_path() / read_symlink(file);
    }
    // TODO: broken symlinks
    assert(is_regular_file(file) || is_directory(file));
    ro_bind(file, root / file.relative_path());
  }

  void full_write_at(int dfd, const char* name, std::string_view data) {
    auto fd = sys::openat(dfd, name, O_WRONLY, 0);
    while (!data.empty()) data.remove_prefix(sys::write(fd, data.data(), data.size()));
    sys::close(fd);
  }
} // namespace detail

struct safe_process {
  process p;
  ivl::linux::owned_file_descriptor cgroup_fd;
  ivl::linux::owned_file_descriptor root_fd;

  // TODO: dont really like this
  ivl::linux::owned_file_descriptor stdout_fd;
  ivl::linux::owned_file_descriptor stderr_fd;
};

// Files don't have to exist.
// If an output file doesn't exist, the mmap_region is empty.
safe_process safe_start(
  process_config pc, const std::vector<std::filesystem::path>& inputs, const std::filesystem::path& wd,
  size_t max_memory, size_t max_cpu_percentage, bool detach_stdin, bool detach_stdout, bool detach_stderr
) {
  // Setting up the cgroup.
  std::string child_cgroup_name = "child.XXXXXXXXXX.slice";
  auto parent_cgroup_fd = sys::open(
    "/sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/ivl-isolation.slice",
    O_RDONLY | O_DIRECTORY | O_CLOEXEC, 0
  );
  while (true) {
    for (int i = 0; i < 10; ++i) child_cgroup_name[6 + i] = '0' + rand() % 10;
    auto fd = ivl::linux::raw_syscalls::openat(parent_cgroup_fd, child_cgroup_name.c_str(), O_RDONLY | O_DIRECTORY, 0);
    if (fd < 0) break;
    sys::close(fd);
  }
  sys::mkdirat(parent_cgroup_fd, child_cgroup_name.c_str(), 0777);
  auto cgroup_fd = sys::openat(parent_cgroup_fd, child_cgroup_name.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC, 0);
  sys::close(parent_cgroup_fd);
  detail::full_write_at(cgroup_fd, "memory.swap.max", "0");
  detail::full_write_at(cgroup_fd, "memory.zswap.max", "0");
  detail::full_write_at(cgroup_fd, "memory.max", std::to_string(max_memory));
  detail::full_write_at(cgroup_fd, "cpu.max", std::format("{}000 100000", max_cpu_percentage));
  pc.cgroup = cgroup_fd;
  std::filesystem::path root = "/dev/shm/mount-point";

  pc.share_fds = true;
  pc.isolate_all = true;
  ivl::linux::owned_file_descriptor tmpfsfd;
  ivl::linux::owned_file_descriptor stdoutfd;
  ivl::linux::owned_file_descriptor stderrfd;

  // bool detach_stdin = true;
  // bool detach_stdout = true;
  // bool detach_stderr = true;

  pc.pre_exec([&] {
    // TODO: man 7 user_namespaces
    detail::full_write_at(AT_FDCWD, "/proc/self/uid_map", "0 1000 1");
    detail::full_write_at(AT_FDCWD, "/proc/self/setgroups", "deny");
    detail::full_write_at(AT_FDCWD, "/proc/self/gid_map", "0 1000 1");

    sys::mount("tmpfs", (char*)root.c_str(), "tmpfs", 0, /*size={} could go here*/ nullptr);
    tmpfsfd = ivl::linux::owned_file_descriptor{(int)sys::open(root.c_str(), O_RDONLY | O_CLOEXEC, 0)};

    for (auto&& input : inputs) detail::replicate(root / "root", wd / input);
    sys::chroot(root.c_str());
    if (detach_stdout)
      stdoutfd = ivl::linux::owned_file_descriptor{(int)sys::open("/stdout", O_CREAT | O_TRUNC | O_RDWR, 0444)};
    if (detach_stderr)
      stderrfd = ivl::linux::owned_file_descriptor{(int)sys::open("/stderr", O_CREAT | O_TRUNC | O_RDWR, 0444)};
    sys::unshare(CLONE_FILES);
    if (detach_stdin) sys::close(0);
    if (detach_stdout) {
      sys::dup2(stdoutfd.get(), 1);
      // CLONE_VM makes close() member function bad
      sys::close(stdoutfd.get());
    }
    if (detach_stderr) {
      sys::dup2(stderrfd.get(), 2);
      // CLONE_VM makes close() member function bad
      sys::close(stderrfd.get());
    }

    sys::chroot("/root");
    sys::mkdir("/tmp", 0777);
    sys::chdir(wd.c_str());
  });

  safe_process ret;

  {
    ret.p = pc.clone_and_exec().unwrap_or_terminate();
    assert(!tmpfsfd.empty());
  }

  ret.root_fd = std::move(tmpfsfd);
  ret.cgroup_fd = ivl::linux::owned_file_descriptor{(int)cgroup_fd};
  ret.stdout_fd = std::move(stdoutfd);
  ret.stderr_fd = std::move(stderrfd);

  return ret;
}

struct safe_run_return {
  int wstatus;
  std::map<std::filesystem::path, ivl::linux::owned_file_descriptor> outputs;
};

// Files don't have to exist.
// If an output file doesn't exist, the mmap_region is empty.
safe_run_return safe_run(
  process_config pc, const std::vector<std::filesystem::path>& inputs,
  const std::vector<std::filesystem::path>& outputs, const std::filesystem::path& wd, size_t max_memory,
  size_t max_cpu_percentage
) {
  auto sp = safe_start(std::move(pc), inputs, wd, max_memory, max_cpu_percentage, false, false, false);

  safe_run_return ret;
  ret.wstatus = sp.p.wait().unwrap_or_terminate();

  // TODO: maybe get some stats from the cgroup (like memory.peak) before removing it
  detail::full_write_at(sp.cgroup_fd.get(), "cgroup.kill", "1");
  // TODO: remove cgroup dir
  // sys::unlinkat(parent_cgroup_fd, child_cgroup_name.c_str(), AT_REMOVEDIR);

  for (auto&& file : outputs) {
    auto fd =
      ivl::linux::raw_syscalls::openat(sp.root_fd.get(), (wd / file).relative_path().c_str(), O_RDONLY | O_CLOEXEC, 0);
    assert(fd >= 0 || fd == -ENOENT);
    ret.outputs[file] = fd >= 0 ? ivl::linux::owned_file_descriptor{(int)fd} : ivl::linux::owned_file_descriptor{};
  }

  return ret;
}
} // namespace ivl
