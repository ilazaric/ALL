#pragma once

#include <ivl/build_system/task_config>
#include <ivl/build_system/task_outcome>
#include <ivl/linux/clone3>
#include <ivl/linux/epoll>
#include <ivl/linux/throwing_syscalls>
#include <ivl/linux/utility>
#include <sys/timerfd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <vector>

namespace ivl::build_system {
struct task_executor {
  linux::epoll_file_descriptor efd;
  std::filesystem::path root_cgroup_dir;

  explicit task_executor(const std::filesystem::path& root_cgroup_dir)
      : efd(linux::throwing_syscalls::semantic, linux::epoll_create_enum::EPOLL_CLOEXEC),
        root_cgroup_dir(root_cgroup_dir) {
    namespace sys = linux::throwing_syscalls;
    auto parent_dir = root_cgroup_dir / "parent.slice";
    if (!exists(parent_dir)) sys::mkdir(parent_dir.c_str(), 0755);
    contract_assert(linux::read_file_slow(parent_dir / "pids.current") == "0\n");
    linux::write_file_slow(parent_dir / "cgroup.procs", "0");
    contract_assert(linux::read_file_slow(parent_dir / "pids.current") == "1\n");

    // TODO: test if root cgroup properly configured

    // try to do some cleanup
    for (auto&& entry : std::filesystem::directory_iterator(root_cgroup_dir))
      if (entry.is_directory()) {
        auto&& path = entry.path();
        if (path.native().ends_with("parent.slice")) continue;
        linux::write_file_slow(path / "cgroup.kill", "1");
        linux::write_file_slow(path / "memory.max", "0");
        linux::raw_syscalls::rmdir(path.c_str());
      }
  }

  struct running_task_state {
    linux::owned_file_descriptor pidfd;
    linux::owned_file_descriptor timerfd;
    linux::owned_file_descriptor stdoutfd;
    linux::owned_file_descriptor stderrfd;
    // can't be a fd, bc we couldn't remove it, can be a number + parent cgroup fd maybe TODO
    std::filesystem::path cgroup_dir;
    // TODO: could replace `task_identifier` use with `task_index`, removing allocation
    std::string task_identifier;
    std::chrono::time_point<std::chrono::steady_clock> start_time_point;
    std::size_t task_index;

    bool is_reaped() const { return task_index == (std::size_t)-1 || pidfd.empty(); }

    void terminate_cgroup() { linux::write_file_slow(cgroup_dir / "cgroup.kill", "1"); }

    // TODO: this should probably be a free function somewhere
    std::map<std::string, std::string> collect_cgroup_files() {
      namespace sys = linux::throwing_syscalls;

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
            auto raw_fd = linux::raw_syscalls::openat(fd.get(), dent->d_name, O_RDONLY, 0);
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

    task_outcome terminate_and_reap() {
      task_index = -1;
      contract_assert(!pidfd.empty());
      namespace sys = linux::throwing_syscalls;
      terminate_cgroup();
      task_outcome outcome;
      outcome.identifier = task_identifier;
      siginfo_t info{};
      sys::waitid(P_PIDFD, pidfd.get(), (siginfo*)&info, WEXITED | __WALL, &outcome.end_rusage);
      contract_assert(info.si_code == CLD_EXITED || info.si_code == CLD_KILLED || info.si_code == CLD_DUMPED);
      outcome.duration = std::chrono::steady_clock::now() - start_time_point;
      outcome.end_cgroup_files = collect_cgroup_files();
      linux::write_file_slow(cgroup_dir / "memory.max", "0");
      linux::raw_syscalls::rmdir(cgroup_dir.c_str());
      // this should evict them from epoll fd
      (void)pidfd.close();
      (void)timerfd.close();
      // TODO: might want to encode si_code too
      outcome.exit_status = info.si_status;
      outcome.stdout = linux::read_file(stdoutfd);
      outcome.stderr = linux::read_file(stderrfd);
      return outcome;
    }

    running_task_state(const running_task_state&) = delete;
    running_task_state(running_task_state&&) = default;

    running_task_state& operator=(const running_task_state&) = delete;
    running_task_state& operator=(running_task_state&&) = default;

    ~running_task_state() {
      if (!is_reaped()) (void)terminate_and_reap();
    }
  };

  std::vector<running_task_state> tasks;
  std::vector<std::size_t> unused_slots;
  // TODO: kinda hate this, if epoll works well enough this can be killed off
  std::map<std::size_t, std::size_t> task_index_to_slot;
  std::size_t active_task_count = 0;
  std::size_t seen_task_count = 0;

  std::filesystem::path create_new_cgroup() {
    namespace sys = linux::throwing_syscalls;
    while (true) {
      auto path = root_cgroup_dir / std::format("child.{:08X}.slice", rand());
      if (exists(path)) {
        if (linux::read_file_slow(path / "memory.max") == "0\n") linux::raw_syscalls::rmdir(path.c_str());
        continue;
      }
      sys::mkdir(path.c_str(), 0755);
      return path;
    }
  }

  void launch_task(
    const task_config& task, std::size_t cpu_max_percentage, std::size_t memory_max, std::chrono::nanoseconds time_limit
  ) {
    namespace sys = linux::throwing_syscalls;

    contract_assert(time_limit > std::chrono::nanoseconds(0));

    auto argv_envp_helper = [](const std::vector<std::string>& seq) {
      std::vector<const char*> ret(seq.size() + 1, nullptr);
      for (std::size_t i = 0; i < seq.size(); ++i) ret[i] = seq[i].c_str();
      return ret;
    };

    auto argv = argv_envp_helper(task.process_argv);
    auto envp = argv_envp_helper(task.process_envp);

    linux::owned_file_descriptor timerfd(sys::timerfd_create(CLOCK_MONOTONIC, /* TODO: TFD_NONBLOCK? */ TFD_CLOEXEC));
    {
      struct itimerspec spec{};
      spec.it_value.tv_sec = (int)(time_limit.count() / 1'000'000'000);
      spec.it_value.tv_nsec = (int)(time_limit.count() % 1'000'000'000);
      sys::timerfd_settime(timerfd.get(), 0, (const __kernel_itimerspec*)&spec, nullptr);
    }

    linux::owned_file_descriptor stdinfd(sys::open("/dev/null", O_RDONLY | O_CLOEXEC, 0));
    linux::owned_file_descriptor stdoutfd(sys::memfd_create("stdout", MFD_CLOEXEC));
    linux::owned_file_descriptor stderrfd(sys::memfd_create("stderr", MFD_CLOEXEC));
    // this is relevant for the post-clone pre-exec lambda
    contract_assert(stdinfd.get() < stdoutfd.get() && stdoutfd.get() < stderrfd.get());
    // so i dont have to switch dup2 calls to fnctl (bc of cloexec)
    contract_assert(stdinfd.get() != 0);
    contract_assert(stdoutfd.get() != 1);
    contract_assert(stderrfd.get() != 2);
    auto cgroup_dir = create_new_cgroup();
    linux::write_file_slow(cgroup_dir / "memory.max", std::to_string(memory_max));
    linux::write_file_slow(cgroup_dir / "cpu.max", std::format("{}000 100000", cpu_max_percentage));
    linux::write_file_slow(cgroup_dir / "memory.swap.max", "0");
    linux::write_file_slow(cgroup_dir / "memory.zswap.max", "0");
    linux::owned_file_descriptor pidfd;
    pid_t ppid = sys::getpid();

    {
      linux::owned_file_descriptor cgroupfd(sys::open(cgroup_dir.c_str(), O_PATH, 0));
      alignas(16) char stack[1ULL << 12];
      clone_args clone3_args{
        // NB: fd table not shared
        .flags = CLONE_CLEAR_SIGHAND | CLONE_VM | CLONE_VFORK | CLONE_INTO_CGROUP | CLONE_PIDFD,
        .pidfd = reinterpret_cast<uint64_t>(&pidfd),
        .stack = reinterpret_cast<uintptr_t>(&stack[0]),
        .stack_size = sizeof(stack),
        .cgroup{(std::uint64_t)cgroupfd.get()},
      };
      linux::generic_clone3(sys::semantic, clone3_args, [&] {
        namespace sys = linux::terminate_syscalls;
        sys::prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
        if (sys::getppid() != ppid) sys::exit_group(1);
        sys::chdir(task.process_working_directory.c_str());
        sys::dup2(stdinfd.get(), 0);
        sys::dup2(stdoutfd.get(), 1);
        sys::dup2(stderrfd.get(), 2);
        sys::close_range(3, (unsigned int)-1, 0);
        sys::execve(task.process_pathname.c_str(), argv.data(), envp.data());
      });
      contract_assert(!pidfd.empty());
    }
    auto start_time_point = std::chrono::steady_clock::now();

    std::size_t task_index = seen_task_count++;
    std::size_t slot_index;
    if (unused_slots.empty()) {
      slot_index = tasks.size();
      tasks.emplace_back(running_task_state{});
    } else {
      slot_index = unused_slots.back();
      unused_slots.pop_back();
    }

    tasks[slot_index] = running_task_state{
      .pidfd = std::move(pidfd),
      .timerfd = std::move(timerfd),
      .stdoutfd = std::move(stdoutfd),
      .stderrfd = std::move(stderrfd),
      .cgroup_dir = std::move(cgroup_dir),
      .task_identifier = task.identifier,
      .start_time_point = start_time_point,
      .task_index = task_index,
    };
    ++active_task_count;

    task_index_to_slot[task_index] = slot_index;

    struct epoll_event event{};
    event.events = EPOLLIN;
    event.data.u64 = task_index;
    efd.ctl_add(sys::semantic, tasks[slot_index].pidfd, event);
    efd.ctl_add(sys::semantic, tasks[slot_index].timerfd, event);
  }

  task_outcome wait_for_death() {
    contract_assert(active_task_count > 0);
    namespace sys = linux::throwing_syscalls;
    struct epoll_event event;
    efd.wait_block_forever(sys::semantic, {&event, &event + 1});
    std::size_t task_index = event.data.u64;
    --active_task_count;
    contract_assert(task_index < seen_task_count);
    contract_assert(task_index_to_slot.contains(task_index));
    auto slot_index = task_index_to_slot[task_index];
    contract_assert(slot_index < tasks.size());
    contract_assert(tasks[slot_index].task_index == task_index);
    unused_slots.push_back(slot_index);
    task_index_to_slot.erase(task_index);
    contract_assert(!task_index_to_slot.contains(task_index));
    contract_assert(task_index_to_slot.size() == active_task_count);
    return tasks[slot_index].terminate_and_reap();
  }
};
} // namespace ivl::build_system
