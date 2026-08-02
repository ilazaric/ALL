#include <ivl/build_system/task_executor>
#include <ivl/build_system/task_fingerprint>
#include <ivl/crypto/blake3>
#include <ivl/linux/utility>
#include <ivl/logger>
#include <ivl/reflection/json>
#include <iostream>
#include <string>

#if 0
std::optional<ivl::build_system::task_fingerprint> load_fingerprint(const std::filesystem::path& path) {
  if (!exists(path)) return std::nullopt;
  return ivl::from_json_string<ivl::build_system::task_fingerprint>(ivl::linux::read_file(path));
}

ivl::build_system::task_fingerprint generate_fingerprint(const ivl::build_system::task_config& config) {
  ivl::build_system::task_fingerprint f{.config = config};
  for (auto&& input : config.inputs)
    f.infos.emplace_back(
      ivl::build_system::task_fingerprint::file_info{
        .path = input,
        .mtime = last_write_time(input),
        .hash = ivl::crypto::blake3::hash(ivl::linux::read_file(input)),
      }
    );
  return f;
}

bool check_fingerprint(
  const ivl::build_system::task_config& config, std::optional<ivl::build_system::task_fingerprint>& of
) post(of.has_value()) {
  if (!of.has_value() || of->config != config) {
    of = generate_fingerprint(config);
    return true;
  }
  // TODO: not correct for dependency on directory, but maybe dont want those
  contract_assert(config.inputs.size() == of->infos.size());
  bool changed = false;
  for (auto& info : of->infos) {
    auto mtime = last_write_time(info.path);
    if (mtime == info.mtime) continue;
    info.mtime = mtime;
    auto hash = ivl::crypto::blake3::hash(ivl::linux::read_file(info.path));
    if (hash == info.hash) continue;
    changed = true;
    info.hash = hash;
  }
  return changed;
}

std::optional<ivl::build_system::task_fingerprint> load_fingerprint(const ivl::build_system::task_config& task) {
  if (!exists(path)) return std::nullopt;
  return ivl::from_json_string<ivl::build_system::task_fingerprint>(ivl::linux::read_file(path));
}

struct blatruc {
  std::vector<ivl::build_system::task_config> tasks;
  std::vector<std::optional<ivl::build_system::task_fingerprint>> fingerprints;
  std::map<std::string, std::size_t> identifier2index;
  std::map<std::filesystem::path, std::size_t> output2index;
  std::vector<bool> checked;
  
  std::vector<std::size_t> execs;
  std::vector<std::size_t> dependency_counts;

  std::size_t register_task(const ivl::build_system::task_config& task) {
    contract_assert(!identifier2index.contains(task.identifier));
    auto index = tasks.size();
    tasks.push_back(task);
    identifier2index[task.identifier] = index;
    for (auto&& output : task.outputs) {
      contract_assert(!output2index.contains(output));
      output2index[output] = index;
    }
    checked.push_back(false);
    fingerprints.push_back(std::nullopt);
    return index;
  }

  void execute_task(std::size_t index) {
    contract_assert(index < tasks.size());
    if (checked[index]) return;
    checked[index] = true;
    fingerprints[index] = load_fingerprint(task);
    const auto& task = tasks[index];
    for (auto&& input : task.inputs)
      update_file_if_managed(input);
    
  }

  void execute_task(std::string_view identifier) {
    auto it = identifier2index.find(identifier);
    contract_assert(it != identifier2index.end());
    execute_task(it->second);
  }

  void update_file(const std::filesystem::path& file) {
    auto it = output2index.find(file);
    contract_assert(it != output2index.end());
    execute_task(it->second);
  }

  bool is_managed(const std::filesystem::path& file) {
    return output2index.contains(file);
  }

  void update_file_if_managed(const std::filesystem::path& file) {
    auto it = output2index.find(file);
    if (it != output2index.end())
      execute_task(it->second);
  }

  void run_execute(ivl::build_system::task_executor& executor);
};

int ivl_main() {
  using namespace std::literals::chrono_literals;

  // requires:
  // mkdir /sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/ivl.slice
  // echo '+cpu +memory +pids' > \
  //   /sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/ivl.slice/cgroup.subtree_control
  ivl::build_system::task_executor executor("/sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/ivl.slice");

  auto tasks = ivl::from_json_string<std::vector<ivl::build_system::task_config>>(
    "[" + ivl::linux::read_file("/home/ilazaric/repos/ALL/build2/task_timestamp.json") + "]"
  );

  blatruc bt;
  for (auto&& task : tasks) bt.register_task(task);

  bt.run_execute(executor);

  std::vector<std::optional<ivl::build_system::task_fingerprint>> fingerprints;

  for (auto&& task : tasks) {
    fingerprints.emplace_back(task.outputs.empty() ? std::nullopt : load_fingerprint(task));
    if (
    executor.launch_task(task);
  }

  while (executor.active_task_count) {
    auto outcome = executor.wait_for_death();
    LOG(outcome.identifier, outcome.info.si_status);
  }

  return 0;
}
// TODO
#endif

int main() {}
