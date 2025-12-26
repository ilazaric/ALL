#include <ivl/build_system/safe_run>
#include <ivl/linux/utils>
#include <ivl/util>
#include <algorithm>
#include <concepts>
#include <print>
#include <ranges>
#include <span>
#include <string_view>

// TODO: if good, extract to ivl/argument_parsing
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

/*
  ........ v-- col: 1    v-- col: 40
  row: 1 | ............... |
  row: 2 | ............... |
  row: 3 ! .....x......... !
                ^-- here
  row: 4 | ............... |
  row: 5 | ............... |
 */

// A line is a sequence of characters terminated by a newline.
// The newline is understood to be part of the line.
void print_context(std::string_view file, size_t loc) {
  // TODO: loc == npos might make sense
  assert(loc < file.size());

  auto find_line_start = [&](size_t loc) { return file.substr(0, loc).rfind('\n') + 1; };
  auto find_line_newline = [&](size_t loc) { return file.find('\n', loc); };

  size_t row = std::ranges::count(file.substr(0, loc), '\n');
  size_t col = loc - (file.substr(0, loc).rfind('\n') + 1);
  size_t row_context = 3;
  size_t row_digit_count = [&] {
    auto x = row + row_context + 1;
    size_t ret = !x;
    while (x) x /= 10, ++ret;
    return ret;
  }();

  std::vector<std::string_view> lines;
  {
    size_t idx = 0;
    while (true) {
      auto nxt = file.find('\n', idx);
      lines.push_back(file.substr(idx, nxt - idx));
      if (nxt == std::string_view::npos) break;
      idx = nxt + 1;
    }
  }

  for (size_t i = row < row_context ? 0 : row - row_context; i <= row; ++i)
    std::println(stderr, "{:{}}: {}", i + 1, row_digit_count, lines[i]);
  std::println(stderr, "{:>{}}", "^--- here", col + row_digit_count + 2 + 9);
  for (size_t i = row + 1; i <= row + row_context && i < lines.size(); ++i)
    std::println(stderr, "{:{}}: {}", i + 1, row_digit_count, lines[i]);
}

// std::vector<std::string> parse_args(std::string_view data) {
//   std::vector<std::string> ret;
//   std::string_view sv; // TODO: rename

// new_arg:
//   while (!sv.empty() && isspace(sv[0])) sv.remove_prefix(1);
//   if (sv.empty()) goto end;

//   ret.emplace_back();
//   auto& arg = ret.back();

// consume_arg:
//   if (sv.empty() || isspace(sv[0])) goto new_arg;

//   if (sv[0] == '\'') {
//     auto loc = sv.find('\'', 1);
//     if (loc == std::string_view::npos) {
//       std::print(
//         stderr, "Malformed argument file\n"
//                 "- character `'` is missing ending conterpart\n"
//       );
//       print_context(data, data.size() - sv.size());
//       sys::exit_group(1);
//     }
//     arg += sv.substr(0, loc).substr(1);
//     sv.remove_prefix(loc + 1);
//     goto consume_arg;
//   }

//   if (sv[0] == '\\') {
//     if (sv.size() == 1) {
//       std::print(
//         stderr, "Malformed argument file\n"
//                 "- character `\` with no character after it\n"
//       );
//       print_context(data, data.size() - sv.size());
//       sys::exit_group(1);
//     }

//     // TODO
//   }

// end:
//   return ret;
// }

int main(int argc, char* argv[]) {
  // print_context(ivl::linux::read_file("run_test.cpp"), 150);
  // return 1;

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

  // bla.add_argument("@", [&](magic_argument_parser::controller& c) {
  //   // This is basically gcc/libiberty/argv.c:buildargv()
  //   auto data = ivl::linux::read_file(c.consume_argument().c_str());
  //   bla.parse(new_args);
  // });

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
