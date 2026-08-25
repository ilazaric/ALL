#pragma once

#include <sys/sysmacros.h>
#include <sys/types.h>

#include <format>
#include <fstream>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace ivl::linux::procfs {

struct MapsEntry {
  void* low_address;
  void* high_address;
  // perms
  bool readable;
  bool writable;
  bool executable;
  bool shared;

  uint64_t offset;
  dev_t dev;
  uint64_t inode;
  std::string pathname;
};

struct Maps : std::vector<MapsEntry> {};

std::optional<Maps> parse_maps(pid_t pid) {
  auto str = [&] {
    std::ifstream inf(std::format("/proc/{}/maps", pid));
    std::stringstream buffer;
    buffer << inf.rdbuf();
    return std::move(buffer.str());
  }();
  auto ishex = [](char c) { return c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F'; };
  auto unhex = [](char c) {
    return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c - 'A' + 10;
  };
  if (str.empty()) return std::nullopt; // probably file doesnt exist
  for (auto& c : str)
    if (c == '\n') c = '\0';
  Maps maps;
  for (auto line_ : std::views::split(str, '\n')) {
    std::string_view line(line_);
    if (line.empty()) continue;
    maps.emplace_back();
    if (line.size() >= 72) maps.back().pathname = line.substr(72);
#define EXPECT(c)                                                                                                      \
  do {                                                                                                                 \
    if (line.empty() || line[0] != c) return std::nullopt;                                                             \
    line.remove_prefix(1);                                                                                             \
  } while (0)
#define READHEX(e)                                                                                                     \
  do {                                                                                                                 \
    auto& r = e;                                                                                                       \
    r = {};                                                                                                            \
    for (int cnt = 0; cnt < 2 * sizeof(r) && !line.empty() && ishex(line[0]); ++cnt) {                                 \
      r = r * 16 + unhex(line[0]);                                                                                     \
      line.remove_prefix(1);                                                                                           \
    }                                                                                                                  \
  } while (0)
#define READPERM(e, y, n)                                                                                              \
  do {                                                                                                                 \
    auto& r = e;                                                                                                       \
    if (line.empty()) return std::nullopt;                                                                             \
    if (line[0] != y && line[0] != n) return std::nullopt;                                                             \
    r = line[0] == y ? true : false;                                                                                   \
  } while (0)
#define READDEC(e)                                                                                                     \
  do {                                                                                                                 \
    auto& r = e;                                                                                                       \
    r = {};                                                                                                            \
    for (int cnt = 0; cnt < 2 * sizeof(r) && !line.empty() && line[0] >= '0' && line[0] <= '9'; ++cnt) {               \
      r = r * 10 + (line[0] - '0');                                                                                    \
      line.remove_prefix(1);                                                                                           \
    }                                                                                                                  \
  } while (0)
    READHEX(*reinterpret_cast<uintptr_t*>(&maps.back().low_address));
    EXPECT('-');
    READHEX(*reinterpret_cast<uintptr_t*>(&maps.back().high_address));
    EXPECT(' ');
    READPERM(maps.back().readable, 'r', '-');
    READPERM(maps.back().writable, 'w', '-');
    READPERM(maps.back().executable, 'x', '-');
    READPERM(maps.back().shared, 's', 'p');
    EXPECT(' ');
    READHEX(maps.back().offset);
    EXPECT(' ');
    {
      unsigned int major, minor;
      READHEX(major);
      EXPECT(':');
      READHEX(minor);
      maps.back().dev = makedev(major, minor);
    }
    EXPECT(' ');
    READDEC(maps.back().inode);
    EXPECT(' ');
#undef EXPECT
#undef READHEX
#undef READPERM
#undef READDEC
  }
  return maps;
}

} // namespace ivl::linux::procfs
