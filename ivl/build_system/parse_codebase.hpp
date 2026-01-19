#pragma once

#include <ivl/linux/throwing_syscalls>
#include <ivl/linux/utils>
#include <ivl/process>
#include <cassert>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

namespace ivl::build_system {
std::string preprocess(const std::filesystem::path& file) {
  linux::owned_file_descriptor outfd{linux::throwing_syscalls::memfd_create("pp-output", MFD_CLOEXEC)};
  process_config cfg;
  cfg.pathname = "/usr/bin/g++";
  cfg.argv = {
    "/usr/bin/g++", "-xc++", "-E",
    // TODO
    "@/home/ilazaric/repos/ALL/build/include_dirs/args.rsp", file.native(), "-o",
    std::format("/proc/{}/fd/{}", linux::throwing_syscalls::getpid(), outfd.get())
  };
  cfg.envp = {{"LC_ALL", "C"}, {"PATH", "/usr/bin"}};
  auto proc = cfg.clone_and_exec().unwrap_or_terminate();
  assert(proc.wait().unwrap_or_terminate() == 0);
  return read_file(outfd);
}

std::vector<std::string> extract_ivl_directives(std::string_view sv) {
  assert(sv.empty() || sv.ends_with('\n'));
  std::vector<std::string> ret;
  while (!sv.empty()) {
    auto newline = sv.find('\n');
    assert(newline != std::string_view::npos);
    auto line = sv.substr(0, newline);
    if (line.starts_with("#pragma IVL")) ret.push_back(std::string(line));
    sv.remove_prefix(newline + 1);
  }
  return ret;
}

  struct target {
    std::filesystem::path source;
    
  };

void parse_ivl(const std::filesystem::path& src) {
  std::vector<std::filesystem::path> headers;
  std::vector<std::filesystem::path> sources;
  for (auto&& entry : std::filesystem::recursive_directory_iterator(src)) {
    if (!entry.is_regular_file()) continue;
    auto&& path = entry.path();
    assert(path.extension() == ".cpp" || path.extension() == ".hpp");
    (path.extension() == ".cpp" ? sources : headers).push_back(path);
  }
}
} // namespace ivl::build_system
