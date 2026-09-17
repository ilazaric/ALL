#pragma once

#warning "TODO"

#include <sys/resource.h>
#include <dirent.h>
#include <map>
#include <string>

namespace ivl::build_system {
// TODO: test this
std::map<std::string, std::string> collect_cgroup_files(const std::filesystem::path& cgroup_dir) {
  using sys = linux::throwing_syscalls;

  linux::owned_file_descriptor fd(sys::open(cgroup_dir.c_str(), O_RDONLY | O_DIRECTORY, 0));
  std::map<std::string, std::string> ret;

  struct my_dirent64 {
    ino64_t d_ino;
    off64_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
  };
  alignas(16) char buf[PATH_MAX * 2];

  while (true) {
    auto count = sys::getdents64(fd.get(), (linux_dirent64*)buf, sizeof(buf));
    if (count == 0) break;

    const char* ptr = buf;
    while (count) {
      auto dent = (const my_dirent64*)ptr;
      contract_assert(dent->d_reclen <= count);

      if (dent->d_type == DT_REG) {
        auto raw_fd = linux::raw_syscalls::openat(fd.get(), dent->d_name, O_RDONLY);
        if (raw_fd >= 0) {
          linux::owned_file_descriptor file_fd(raw_fd);
          ret[dent->d_name] = linux::read_file_slow(file_fd);
        }
      }

      ptr += dent->d_reclen;
      count -= dent->d_reclen;
    }
  }

  return ret;
}

struct running_task {
  linux::owned_file_descriptor pidfd;
  std::filesystem::path cgroup_dir;
};

running_task launch_task(const task& t, size_t max_cpu, std::size_t max_memory) { todo(); }

struct build_ecosystem {
  std::vector<build_task> tasks;

  std::map<std::filesystem::path, std::size_t> output_to_task;

  

  void add_task(build_task&& task) {
    auto index = tasks.size();
    bool invalidate_self = false;
    for (const auto& output : task.outputs) {
      auto& output_index = output_to_task[output];
      if (output_index == EMPTY) {
        output_index = index + 1;
        continue;
      }
      invalidate_self = true;
      if (output_index != DEAD) invalidate_self(output_index - 1);
    }
    tasks.emplace_back(std::move(task));
    if (invalidate_self) invalidate_task(index);
  }
};
} // namespace ivl::build_system
