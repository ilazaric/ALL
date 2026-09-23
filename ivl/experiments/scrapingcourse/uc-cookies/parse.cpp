#include <ivl/format>
#include <ivl/linux/utility>
#include <ivl/logger>
#include <ivl/reflection/json>
#include <ivl/stl/string>
#include <ivl/utility>
#include <map>
#include <set>

struct args {
  std::filesystem::path log;
};

pid_t parse_pid(std::string_view sv) {
  pid_t pid;
  auto ret = std::from_chars(sv.data(), sv.data() + sv.size(), pid);
  ret&& ret.ptr == sv.data() + sv.size() || ivl::panic("failed to parse pid: {:?}", sv);
  return pid;
}

pid_t consume_pid(std::string_view& sv) {
  auto space = sv.find(' ');
  space == std::string_view::npos&& ivl::panic("line doesn't start with pid: {:?}", sv);
  auto pid_sv = sv.substr(0, space);
  sv.remove_prefix(space + 1);
  return parse_pid(pid_sv);
}

void check_syscall(std::string_view syscall, std::string_view line) {
  syscall.empty() && ivl::panic("syscall name empty: {:?}", line);
  for (auto c : syscall)
    if (!std::isalnum(c) && c != '_')
      ivl::panic("syscall name has weird characters\nsyscall: {:?}\ncharacter: {:?}\nline: {:?}", syscall, c, line);
}

int ivl_main(const args& args) {
  const auto& log = args.log;
  auto contents = ivl::linux::read_file(log);
  LOG(contents.size());
  auto lines = ivl::split_view(contents, "\n");
  LOG(lines.size());

  std::set<pid_t> pids;
  std::map<pid_t, std::vector<std::string_view>> execs;
  std::map<pid_t, std::string_view> pending_execs;
  std::map<pid_t, std::string_view> pending_forks;
  std::map<pid_t, pid_t> parents;
  std::map<std::string, size_t> syscall_counts;
  std::map<pid_t, std::string_view> cause_of_death;
  std::set<pid_t> threads;

  pid_t root;
  {
    std::string_view sv(contents);
    root = consume_pid(sv);
  }

  for (std::string_view line : lines) {
    if (line.empty()) continue;

    pid_t pid = consume_pid(line);
    pids.insert(pid);

    if (line.starts_with("<... ")) {
      line.remove_prefix(5);
      auto space = line.find(' ');
      space == std::string_view::npos&& ivl::panic("missing space: {:?}", line);
      auto syscall = line.substr(0, space);
      if (syscall == "???") continue;
      check_syscall(syscall, line);

      if (syscall == "clone" || syscall == "clone3" || syscall == "vfork") {
        auto eq = line.rfind('=');
        eq == std::string_view::npos&& ivl::panic("missing '=': {:?}", line);
        auto eqsv = line.substr(eq + 1);
        eqsv.starts_with(' ') || ivl::panic("missing ' ': {:?}", eqsv);
        eqsv.remove_prefix(1);
        eqsv.starts_with('-') && ivl::panic("unexpected negative: {:?}", eqsv);
        auto child = parse_pid(eqsv);
        parents.contains(child) &&
          ivl::panic("child {} with multiple parents, old={}, new={}", child, parents[child], pid);
        parents[child] = pid;
        if (pending_forks[pid].contains("CLONE_THREAD")) threads.insert(child);
        continue;
      }

      if (syscall == "execve") {
        auto eq = line.rfind('=');
        eq == std::string_view::npos&& ivl::panic("missing '=': {:?}", line);
        auto eqsv = line.substr(eq + 1);
        eqsv.starts_with(' ') || ivl::panic("missing ' ': {:?}", eqsv);
        eqsv.remove_prefix(1);
        if (eqsv.starts_with('-')) continue;
        execs[pid].push_back(pending_execs[pid]);
        continue;
      }

      continue;
    }

    if (line.starts_with("+++ exited with ")) {
      line.remove_prefix(16);
      line.ends_with(" +++") || ivl::panic("weird line {:?}", line);
      line.remove_suffix(4);
      cause_of_death[pid] = line;
      continue;
    }

    if (line.starts_with("--- SIG")) continue;
    if (line.starts_with("+++ superseded by execve in pid")) continue;

    if (line.starts_with("+++ killed by ")) {
      line.remove_prefix(14);
      line.ends_with(" +++") || ivl::panic("weird line {:?}", line);
      line.remove_suffix(4);
      cause_of_death[pid] = line;
      continue;
    }

    auto paren = line.find('(');
    if (paren != std::string_view::npos) {
      auto syscall = line.substr(0, paren);
      if (syscall == "???") continue;
      check_syscall(syscall, line);

      ++syscall_counts[std::string(syscall)];

      if (syscall == "execve") {
        if (line.ends_with("<unfinished ...>")) {
          pending_execs[pid] = line;
          continue;
        }

        auto eq = line.rfind('=');
        eq == std::string_view::npos&& ivl::panic("missing '=': {:?}", line);
        auto eqsv = line.substr(eq + 1);
        eqsv.starts_with(' ') || ivl::panic("missing ' ': {:?}", eqsv);
        eqsv.remove_prefix(1);
        if (eqsv.starts_with('-')) continue;

        execs[pid].push_back(line);
        continue;
      }

      if (syscall == "clone" || syscall == "clone3" || syscall == "vfork") {
        if (line.ends_with("<unfinished ...>")) {
          pending_forks[pid] = line;
          continue;
        }
        auto eq = line.rfind('=');
        eq == std::string_view::npos&& ivl::panic("missing '=': {:?}", line);
        auto eqsv = line.substr(eq + 1);
        eqsv.starts_with(' ') || ivl::panic("missing ' ': {:?}", eqsv);
        eqsv.remove_prefix(1);
        if (eqsv.starts_with('-')) continue;
        auto child = parse_pid(eqsv);
        parents.contains(child) &&
          ivl::panic("child {} with multiple parents, old={}, new={}", child, parents[child], pid);
        parents[child] = pid;
        if (line.contains("CLONE_THREAD")) threads.insert(child);
      }

      continue;
    }

    ivl::panic("dont understand this: {:?}", line);
  }

  LOG(ivl::fmt::format("{}", pids));
  LOG(dump(ivl::to_json(syscall_counts), 2));

  if (parents.contains(root)) ivl::fmt::println("ERROR: root pid {} has parent {}", root, parents[root]);

  size_t orphans = 0;
  for (auto pid : pids)
    if (pid != root && !parents.contains(pid)) {
      ivl::fmt::println("ERROR: pid {} has no parent", pid);
      ++orphans;
    }

  if (orphans) ivl::panic("ERROR: total orphans: {}", orphans);

  for (auto pid : pids)
    if (!cause_of_death.contains(pid)) ivl::fmt::println("ERROR: unknown cause of death for {}", pid);

  std::map<pid_t, std::vector<pid_t>> children;
  for (auto pid : pids)
    if (pid != root) children[parents[pid]].push_back(pid);

  std::map<pid_t, size_t> exec_counts;
  for (auto pid : pids) exec_counts[pid] = execs[pid].size();
  auto populate_exec_counts = [&](this const auto& self, pid_t pid) -> void {
    for (auto child : children[pid]) {
      self(child);
      exec_counts[pid] += exec_counts[child];
    }
  };
  populate_exec_counts(root);

  auto pid = 402428;
  while (pid != root) {
    ivl::fmt::print("{}", pid);
    for (auto&& ex : execs[pid]) ivl::fmt::print(" -> {:?}", ivl::split_view(ex, "\"")[1]);
    ivl::fmt::println("");
    pid = parents[pid];
  }

  for (auto pid : pids)
    if (!execs[pid].empty() && threads.contains(pid)) ivl::fmt::println("ERROR: exec from thread {}", pid);

  std::set<pid_t> visited;
  auto recurse = [&](this const auto& self, pid_t pid, int depth) -> void {
    visited.insert(pid);

    bool is_process = !threads.count(pid);

    // if (exec_counts[pid] != 0)
    {
      ivl::fmt::print(
        "({: <{}}) ({}) {: <{}}{}", cause_of_death[pid], 10, threads.contains(pid) ? "...thrd" : "PROCESS", "", depth,
        pid
      );
      if (is_process)
        for (auto&& ex : execs[pid]) ivl::fmt::print(" -> {:?}", ivl::split_view(ex, "\"")[1]);
      ivl::fmt::println("");
    }

    for (auto child : children[pid]) self(child, depth + 2 * is_process);
  };

  recurse(root, 0);
  if (visited.size() != pids.size()) ivl::fmt::println("ERROR: didnt visit everything");

  return 0;
}
