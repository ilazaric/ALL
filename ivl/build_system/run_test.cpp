#include <ivl/build_system/safe_run>
#include <ivl/util>
#include <algorithm>
#include <concepts>
#include <print>
#include <ranges>
#include <span>
#include <string_view>

struct magic_argument_parser {
  struct controller;

  std::map<std::string, std::function<void(controller&)>> callbacks;

  struct controller {
    bool end_parsing = false;
    std::function<std::string()> consume_argument;
  };

  void add_argument(std::string_view name, std::invocable<controller&> auto&& callback) {
    assert(callbacks.try_emplace(std::string(name), FWD(callback)).second);
  }

  void add_argument(std::string_view name, std::invocable<> auto&& callback) {
    add_argument(name, [cb = FWD(callback)](controller&) { cb(); });
  }

  void parse(auto&& rg) {
    auto it = std::ranges::begin(rg);
    auto end = std::ranges::end(rg);
    auto empty = [&] { return it == end; };

    controller c;
    c.consume_argument = [&] {
      assert(!empty());
      std::string ret = *it;
      ++it;
      return ret;
    };

    while (!empty() && !c.end_parsing) {
      std::string str = *it;
      ++it;
      auto cbit = callbacks.find(str);
      assert(cbit != callbacks.end());
      cbit->second(c);
    }
  }
};

int main(int argc, char* argv[]) {
  std::span<char*> args{argv + 1, argv + argc};
  std::span<char*> passthrough;

  {
    auto it = std::ranges::find(args, "--", ivl::util::convert<std::string_view>);
    if (it == args.end()) {
      std::cerr << "ERROR: no `--` separator\n";
      return 1;
    }
    auto idx = it - args.begin();
    passthrough = args.subspan(idx + 1);
    args = args.subspan(0, idx);
  }

  assert(!passthrough.empty());

  ivl::process_config pc;
  pc.pathname = passthrough[0];
  pc.argv = std::ranges::to<std::vector<std::string>>(passthrough);

  std::vector<std::filesystem::path> inputs;
  size_t time_limit = 5000;
  std::filesystem::path wd;
  size_t max_memory = 0;
  size_t max_cpu_percentage = 0;
  bool verbose = false;

  magic_argument_parser bla;

  bla.add_argument("--env", [&](magic_argument_parser::controller& c) {
    auto key = c.consume_argument();
    auto value = c.consume_argument();
    assert(pc.envp.try_emplace(std::move(key), std::move(value)).second);
  });

  bla.add_argument("--dependency", [&](magic_argument_parser::controller& c) {
    inputs.emplace_back(c.consume_argument());
  });

  bla.add_argument("--time-limit-ms", [&](magic_argument_parser::controller& c) {
    time_limit = std::stoi(c.consume_argument());
  });

  bla.add_argument("--max-memory", [&](magic_argument_parser::controller& c) {
    max_memory = std::stoi(c.consume_argument());
  });

  bla.add_argument("--max-cpu-percentage", [&](magic_argument_parser::controller& c) {
    max_cpu_percentage = std::stoi(c.consume_argument());
  });

  bla.add_argument("--wd", [&](magic_argument_parser::controller& c) { wd = c.consume_argument(); });

  bla.add_argument("--verbose", [&] { verbose = true; });

  bla.parse(args);

  assert(!wd.empty());
  wd = canonical(wd);

  auto s = ivl::safe_start(
    std::move(pc), inputs, wd, max_memory, max_cpu_percentage,
    // false, false, false
    true, !verbose, !verbose
  );

  auto wstatus = s.p.wait().unwrap_or_terminate();
  if (wstatus == 0) {
    std::println("TEST PASSED");
    return 0;
  }

  std::println("TEST FAILED WITH WSTATUS {}", wstatus);

  if (verbose) {
    std::println("STDOUT AND STDERR WERE ALREADY DUMPED BY `--verbose`");
  } else {
    auto dump = [](ivl::linux::file_descriptor fd) {
      assert(!fd.empty());
      struct stat statbuf;
      ivl::sys::fstat(fd.get(), &statbuf);
      if (statbuf.st_size == 0) return;
      auto ptr = ivl::sys::mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd.get(), 0);
      std::string_view data((const char*)ptr, statbuf.st_size);
      while (true) {
        auto loc = data.find('\n');
        auto line = data.substr(0, loc);
        std::println("+ {}", line);
        if (loc == std::string_view::npos) break;
        data.remove_prefix(loc + 1);
      }
      ivl::sys::munmap(ptr, statbuf.st_size);
    };
    std::println("STDOUT:");
    dump(s.stdout_fd);
    std::println("STDERR:");
    dump(s.stderr_fd);
  }

  return 1;
}
