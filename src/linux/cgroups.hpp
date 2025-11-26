#pragma once

// Sources:
// https://labs.iximiuz.com/tutorials/controlling-process-resources-with-cgroups
// https://docs.kernel.org/admin-guide/cgroup-v2.html
// https://docs.redhat.com/en/documentation/red_hat_enterprise_linux/8/html/managing_monitoring_and_updating_the_kernel/using-cgroups-v2-to-control-distribution-of-cpu-time-for-applications_managing-monitoring-and-updating-the-kernel
// https://man7.org/linux/man-pages/man7/cgroups.7.html

#include <filesystem>
#include <cassert>
#include <print>
#include <cstdio>

namespace ivl::linux::cgroups {

  std::filesystem::path mount_point() {
    // this one is under ilazaric user, simpler to play around
    // TODO
    return "/sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/ivl_testing_cgroup";
    // return "/sys/fs/cgroup";
  }

  std::filesystem::path cgroup_dir(std::filesystem::path cgroup) {
    assert(cgroup.is_relative());
    auto abs_path = (mount_point() / cgroup).lexically_normal();
    assert(abs_path.native().starts_with(mount_point().native() + "/"));
    return abs_path;
  }

  void create(std::filesystem::path path) {
    auto dir = cgroup_dir(path);
    auto b = create_directory(dir);
    assert(b);
  }

  void destroy(std::filesystem::path path) {
    auto b = remove(cgroup_dir(path));
    assert(b);
  }

} // namespace ivl::linux::cgroups
