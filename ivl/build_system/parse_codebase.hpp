#pragma once

#include <ivl/crypto/blake3>
#include <ivl/linux/throwing_syscalls>
#include <ivl/linux/utils>
#include <ivl/process>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

namespace ivl::build_system {
std::optional<std::string> preprocess(const std::filesystem::path& file) {
  linux::owned_file_descriptor outfd{linux::throwing_syscalls::memfd_create("pp-output", MFD_CLOEXEC)};
  process_config cfg;
  cfg.pathname = "/opt/GCC/bin/g++";
  cfg.argv = {
    cfg.pathname, "-xc++", "-E", "-fdiagnostics-plain-output",
    // TODO
    "@/home/ilazaric/repos/ALL/build/include_dirs/args.rsp", file.native(), "-o",
    std::format("/proc/{}/fd/{}", linux::throwing_syscalls::getpid(), outfd.get())
  };
  cfg.envp = {{"LC_ALL", "C"}, {"PATH", "/opt/GCC/bin"}};
  cfg.pre_exec([] {
    auto fd = linux::terminate_syscalls::open("/dev/null", O_WRONLY, 0);
    if (fd == 2) return;
    linux::terminate_syscalls::dup2(fd, 2);
    linux::terminate_syscalls::close(fd);
  });
  auto proc = cfg.clone_and_exec().unwrap_or_terminate();
  if (proc.wait().unwrap_or_terminate() == 0) {
    // LOG(1, file.native());
    auto ret = read_file(outfd);
    // LOG(2);
    return ret;
  } else {
    std::println(stderr, "ERROR: file `{}` failed to preprocess", file.native());
    return std::nullopt;
  }
}

struct ivl_directive {
  std::filesystem::path file;
  std::string pragma;
};

std::vector<ivl_directive> extract_ivl_directives(std::string_view sv) {
  if (!sv.empty() && !sv.ends_with('\n')) panic("Preprocessed file doesn't end with a newline\n```\n{}```", sv);
  std::vector<ivl_directive> ret;
  std::filesystem::path current_file;
  while (!sv.empty()) {
    auto newline = sv.find('\n');
    if (newline == std::string_view::npos) panic("Missing newline\n```\n{}```", sv);
    auto line = sv.substr(0, newline);
    sv.remove_prefix(newline + 1);
    if (line.starts_with("# ")) {
      line.remove_prefix(2);
      auto first_space = line.find(' ');
      if (first_space == std::string_view::npos) panic("Missing space in linemarker\n```\n{}\n```", line);
      auto second_space = line.find(' ', first_space + 1);
      current_file = line.substr(0, second_space).substr(first_space);
    }
    auto expected_prefix = std::string_view("#pragma IVL");
    if (line.starts_with(expected_prefix)) {
      if (line.size() == expected_prefix.size() || line[expected_prefix.size()] != ' ')
        panic("Malformed pragma directive\n```\n{}\n```", line);
      ret.emplace_back(current_file, std::string(line.substr(expected_prefix.size() + 1)));
    }
  }
  return ret;
}

// understands `"` and `\"`
std::vector<std::string> parse_pragma_arg(std::string_view sv) {
  std::vector<std::string> ret;
  while (!sv.empty()) {
    while (!sv.empty() && isspace(sv[0])) sv.remove_prefix(1);
    if (sv.empty()) break;

    if (sv[0] != '"') {
      auto space = sv.find(' ');
      auto current = sv.substr(0, space);
      if (current.contains('"')) panic("Cannot have quote character in a non-quoted string\n```\n{}\n```", sv);
      ret.emplace_back(std::string(current));
      sv.remove_prefix(current.size());
      continue;
    }

    auto endquote = sv.find('"', 1);
    while (endquote != std::string_view::npos && sv[endquote - 1] == '\\') endquote = sv.find('"', endquote + 1);

    if (endquote == std::string_view::npos) panic("Missing terminating quote\n```\n{}\n```", sv);
    if (endquote != sv.size() - 1 && sv[endquote + 1] != ' ')
      panic("Terminating quote not followed by space\n```\n{}\n```", sv);
    ret.emplace_back(std::string(sv.substr(0, endquote).substr(1)));
    sv.remove_prefix(endquote + 1);
  }
  return ret;
}

struct source_target {
  std::filesystem::path file;
  std::vector<std::string> add_compiler_flags;
  std::vector<std::string> add_compiler_flags_tail;
  std::vector<std::string> dependencies;
  std::vector<std::string> test_dependencies;
  bool has_reg_variant;
  bool has_test_variant;
};

// call this with source_copy dir
std::vector<source_target> parse_ivl(const std::filesystem::path& src) {
  std::vector<std::filesystem::path> headers;
  std::vector<std::filesystem::path> sources;
  for (auto&& entry : std::filesystem::recursive_directory_iterator(src)) {
    if (!entry.is_regular_file()) continue;
    auto&& path = entry.path();
    if (path.extension() != ".cpp" && path.extension() != ".hpp") panic("Bad file extension: {}", path.native());
    (path.extension() == ".cpp" ? sources : headers).push_back(path);
  }

  size_t total = 0;
  std::vector<std::pair<std::chrono::duration<double>, std::filesystem::path>> timings;

  std::vector<source_target> targets;
  for (auto&& source : sources) {
    auto before = std::chrono::high_resolution_clock::now();
    auto opp = preprocess(source);
    auto after = std::chrono::high_resolution_clock::now();
    timings.emplace_back(after - before, source);
    if (!opp) continue;
    auto&& pp = *opp;
    total += pp.size();
    auto ivl_directives = extract_ivl_directives(pp);

    targets.emplace_back();
    auto& target = targets.back();
    target.file = source;

    target.has_reg_variant = source.extension() == ".cpp";
    target.has_test_variant = false;

    for (auto&& directive : ivl_directives) {
      auto pieces = parse_pragma_arg(directive.pragma);
      if (pieces.empty()) panic("Missing command in pragma:\n```\n{}\n```", directive.pragma);
      auto command = pieces[0];
      pieces.erase(pieces.begin());

      if (command == "add_compiler_flags") {
        target.add_compiler_flags.insert_range(target.add_compiler_flags.end(), pieces);
      } else if (command == "add_compiler_flags_tail") {
        target.add_compiler_flags_tail.insert_range(target.add_compiler_flags_tail.end(), pieces);
      } else if (command == "add_dependencies") {
        target.dependencies.insert_range(target.dependencies.end(), pieces);
      } else if (command == "add_test_dependencies") {
        target.test_dependencies.insert_range(target.test_dependencies.end(), pieces);
      } else if (command == "has_test_variant") {
        if (!pieces.empty()) panic("`has_test_variant` takes no arguments\n```\n{}\n```", directive.pragma);
        target.has_test_variant = true;
      } else if (command == "test_only") {
        if (!pieces.empty()) panic("`test_only` takes no arguments\n```\n{}\n```", directive.pragma);
        target.has_test_variant = true;
        target.has_reg_variant = false;
      } else {
        std::println(stderr, "ERROR: file `{}` has unrecognized IVL directive:", source.native());
        std::println(stderr, "ERROR: directive: {}", directive.pragma);
        std::println(stderr, "ERROR: from file: {}", directive.file.native());
        targets.pop_back();
        break;
      }
    }
  }

  LOG(total);

  std::ranges::sort(timings, std::greater<void>{});
  std::chrono::duration<double> total_duration{};
  for (auto&& [duration, file] : timings) {
    LOG(duration, file);
    total_duration += duration;
  }
  LOG(total_duration);

  return targets;
}
} // namespace ivl::build_system
