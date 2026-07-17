#include <ivl/build_system/task_executor>
#include <ivl/logger>
#include <ivl/reflection/json>
#include <iostream>
#include <string>

int ivl_main() {
  using namespace std::literals::chrono_literals;

  // requires:
  // mkdir /sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/ivl.slice
  // echo '+cpu +memory +pids' > \
  //   /sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/ivl.slice/cgroup.subtree_control
  ivl::build_system::task_executor executor("/sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/ivl.slice");

  auto bash = [](std::string_view id, std::string_view arg) {
    return ivl::build_system::task_config{
      .identifier = std::string("BASH ") + id,
      .process_working_directory = "/tmp",
      .process_pathname = "/usr/bin/bash",
      .process_argv{
        "/usr/bin/bash",
        "-c",
        std::string(arg),
      },
      .process_envp{
        "LC_ALL=C",
        "PATH=/usr/bin",
      },
    };
  };

  auto cxx = [](std::string_view id, std::string_view input, std::string_view output) {
    return ivl::build_system::task_config{
      .identifier = std::string("CXX ") + id,
      .process_working_directory = "/tmp",
      .process_pathname = "/opt/GCC/bin/g++",
      .process_argv{
        "/opt/GCC/bin/g++",
        "-Wl,-rpath=/opt/GCC/lib64",
        "-std=c++26",
        "-O3",
        std::string(input),
        "-o",
        std::string(output),
      },
      .process_envp{
        "LC_ALL=C",
        "PATH=/opt/GCC/bin:/usr/bin",
      },
    };
  };

  if (0) {
    executor.launch_task(bash("fd-check", "echo STDOUT; echo STDERR > /dev/stderr"));
    executor.launch_task(bash("ls-root", "ls /"));
    executor.launch_task(bash("df-root", "df /"));
    executor.launch_task(bash("pwd", "pwd"));

    {
      auto task = bash("stalling-out", "sleep 20");
      task.time_limit = 1s;
      executor.launch_task(task);
    }

    while (executor.active_task_count) {
      auto outcome = executor.wait_for_death();
      std::cout << ivl::to_json(outcome).dump(2) << std::endl;
    }
  }

  if (1) {
    auto start_tp = std::chrono::steady_clock::now();

    std::vector<ivl::build_system::task_config> tasks;
    std::vector<ivl::build_system::task_outcome> outcomes;
    for (int i = 0; i < 20; ++i) {
      tasks.push_back(cxx(
        std::to_string(i), "/home/ilazaric/repos/ALL/ivl/build_system/test_program.cpp",
        "/home/ilazaric/repos/ALL/ivl/build_system/outputs/test_program." + std::to_string(i)
      ));
      tasks.back().cpu_max_percentage = 50;
      tasks.back().time_limit = 20s;
      outcomes.emplace_back();
    }

    int j = 0;
    for (int i = 0; i < 20; ++i) {
      while (executor.active_task_count >= 4) outcomes[j++] = executor.wait_for_death();
      executor.launch_task(tasks[i]);
    }
    while (executor.active_task_count) outcomes[j++] = executor.wait_for_death();
    contract_assert(j == 20);

    auto end_tp = std::chrono::steady_clock::now();
    LOG(end_tp - start_tp);
    for (auto&& outcome : outcomes) LOG(outcome.identifier, outcome.exit_status, outcome.duration);
  }

  return 0;
}
