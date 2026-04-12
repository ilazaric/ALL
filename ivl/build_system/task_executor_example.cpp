#include <ivl/build_system/task_executor>
#include <ivl/reflection/json>
#include <iostream>

int ivl_main() {
  using namespace std::literals::chrono_literals;

  ivl::build_system::task_executor executor("/sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/ivl.slice");

  auto bash = [](std::string_view id, std::string_view arg) {
    return ivl::build_system::task_config{
      .identifier = std::string("BASH ") + id,
      .process_working_directory = std::filesystem::current_path(),
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

  executor.launch_task(bash("fd-check", "sleep 1; echo STDOUT > /dev/stdout; echo STDERR > /dev/stderr"), 100, 1'000'000'000, 10s);
  executor.launch_task(bash("ls-root", "sleep 1; ls /"), 100, 1'000'000'000, 10s);
  executor.launch_task(bash("df-root", "sleep 1; df /"), 100, 1'000'000'000, 10s);

  while (executor.active_task_count) {
    auto outcome = executor.wait_for_death();
    std::cout << ivl::to_json(outcome).dump(2) << std::endl;
  }

  return 0;
}
