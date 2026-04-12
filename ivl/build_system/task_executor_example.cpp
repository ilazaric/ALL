#include <ivl/build_system/task_executor>
#include <ivl/reflection/json>
#include <iostream>

int ivl_main() {
  using namespace std::literals::chrono_literals;

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

  executor.launch_task(bash("fd-check", "echo STDOUT; echo STDERR > /dev/stderr"), 100, 1'000'000'000, 10s);
  executor.launch_task(bash("ls-root", "ls /"), 100, 1'000'000'000, 10s);
  executor.launch_task(bash("df-root", "df /"), 100, 1'000'000'000, 10s);
  executor.launch_task(bash("pwd", "pwd"), 100, 1'000'000'000, 10s);

  executor.launch_task(bash("stalling-out", "sleep 20"), 100, 1'000'000'000, 1s);

  while (executor.active_task_count) {
    auto outcome = executor.wait_for_death();
    std::cout << ivl::to_json(outcome).dump(2) << std::endl;
  }

  return 0;
}
