#include <ivl/linux/cgroups>
#include <ivl/process>
#include <ivl/stl/string>
#include <print>
#include <cstdio>
#include <cstring>
#include <atomic>
#include <thread>

// IVL add_compiler_flags("-static -flto")
// IVL add_compiler_flags_tail("-pthread")

int main() {
  using namespace ivl::linux::cgroups;

  std::filesystem::path topcg = "distrib";
  auto workercg1 = topcg / "worker1";
  auto workercg2 = topcg / "worker2";

  auto write = [](auto&& cg, auto&& file, auto&& txt) {
    auto ff = cgroup_dir(cg) / file;
    auto f = std::fopen(ff.native().c_str(), "w");
    assert(f);
    std::println(f, "{}", txt);
    auto r = std::fclose(f);
    if (r != 0) {
      auto e = errno;
      std::println("{} {} {}", cg.native(), file, txt);
      std::println("err: {}", std::strerror(e));
      std::println("ff: {}", ff.native());
      assert(false);
    }
  };

  if (exists(cgroup_dir(workercg1))) destroy(workercg1);
  if (exists(cgroup_dir(workercg2))) destroy(workercg2);
  if (exists(cgroup_dir(topcg))) destroy(topcg);

  auto create_init = [&](auto cg, auto memmax, auto cpumax) {
    create(cg);
    write(cg, "memory.max", memmax);
    write(cg, "memory.swap.max", "0");
    write(cg, "memory.zswap.max", "0");
    write(cg, "cpu.max", cpumax);
  };

  create_init(topcg, "500M", "200000 100000");
  write(topcg, "cgroup.subtree_control", "+cpu +memory");
  create_init(workercg1, "100M", "100000 100000");
  create_init(workercg2, "30M", "100000 100000");
  write(workercg1, "memory.oom.group", "1");
  write(workercg2, "memory.oom.group", "1");

  std::atomic<size_t> global_job_index;

  std::println("{:.>30} {:.>10} {:.>10} {:.>10}", "cgroup", "job_index", "memory", "exit_code");
  
  auto thread_lambda = [&] (std::filesystem::path cg) {
    ivl::process_config cfg;
    cfg.pathname = "/home/ilazaric/repos/ALL/src/linux/memory_consumer";
    cfg.argv = {cfg.pathname, ""};
    cfg.pre_exec_setup = [&] {
      write(cg, "cgroup.procs", std::to_string(::getpid()));
    };
    while (true) {
      auto job_index = global_job_index++;
      auto mem = (job_index % 64) * (1ULL << 20);
      cfg.argv[1] = std::to_string(mem);
      auto proc = cfg.clone_and_exec().unwrap_or_terminate();
      auto w = proc.wait().unwrap_or_terminate();
      std::println("{:.>30} {:.>10} {:.>10} {:.>10}", cg.native(), job_index, mem, w*100);
    }
  };
  std::thread t1{thread_lambda, workercg1};
  std::thread t2{thread_lambda, workercg2};

  t1.join();
  t2.join();
}
