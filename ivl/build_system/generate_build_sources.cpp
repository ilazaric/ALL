#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

// This program has to be buildable without any ivl headers accessible,
// because this program is the one that sets up the directory hierarchy of headers.

// TODO: check symlinks (want to ignore them)
// TODO: arg parsing, --help, --verbose-{x,y,z}

std::vector<std::filesystem::path> find_sources(const std::filesystem::path& dir) {
  std::vector<std::filesystem::path> ret;
  for (auto&& entry : std::filesystem::recursive_directory_iterator(dir)) {
    if (!entry.is_regular_file()) continue;
    auto&& p = entry.path();
    if (p.extension() == ".cpp" || p.extension() == ".hpp") ret.emplace_back(p);
  }
  return ret;
}

int main() {
  auto root = std::filesystem::canonical("/proc/self/exe");
  while (!exists(root / ".git")) {
    assert(root.has_parent_path());
    root = root.parent_path();
  }
  std::cerr << "repository root: " << root << std::endl;

  std::vector<std::filesystem::path> files = find_sources(root / "ivl");

  auto build_dir = root / "build";
  if (!exists(build_dir)) create_directory(build_dir);

  auto copy_dir = build_dir / "source_copy";
  if (!exists(copy_dir)) create_directory(copy_dir);

  {
    for (auto&& existing : find_sources(copy_dir)) {
      auto original = root / existing.lexically_relative(copy_dir);
      if (!exists(original) || last_write_time(original) != last_write_time(existing)) remove(existing);
    }

    for (auto&& file : files) {
      auto target = copy_dir / file.lexically_relative(root);
      if (exists(target)) {
        file = target;
        continue;
      }
      create_directories(target.parent_path());

      constexpr char added_prefix[] = "#line 1 \"";
      constexpr char added_suffix[] = "\"\n";
      size_t added_length = sizeof(added_prefix) - 1 + file.native().size() + sizeof(added_suffix) - 1;

      auto prevfd = open(file.native().c_str(), O_RDONLY, 0);
      assert(prevfd != -1);
      struct stat statbuf;
      assert(-1 != fstat(prevfd, &statbuf));
      size_t new_size = statbuf.st_size + added_length;
      auto prevmap = statbuf.st_size ? mmap(nullptr, statbuf.st_size, PROT_READ, MAP_PRIVATE, prevfd, 0) : nullptr;
      if (prevmap == MAP_FAILED) {
        auto e = errno;
        std::cout << "file: " << file << std::endl;
        std::cout << "target: " << target << std::endl;
        std::cout << "statbuf.st_size: " << statbuf.st_size << std::endl;
        std::cout << "err: " << e << std::endl;
        assert(false);
      }

      auto fd = open(target.native().c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
      assert(fd != -1);
      assert(-1 != ftruncate(fd, new_size));
      auto map = mmap(nullptr, new_size, PROT_WRITE, MAP_SHARED, fd, 0);
      if (map == MAP_FAILED) {
        auto e = errno;
        std::cout << "file: " << file << std::endl;
        std::cout << "target: " << target << std::endl;
        std::cout << "err: " << e << std::endl;
        assert(false);
      }
      char* ptr = (char*)map;
      auto wr = [&](std::string_view sv) {
        memcpy(ptr, sv.data(), sv.size());
        ptr += sv.size();
      };
      wr(added_prefix);
      wr(file.native());
      wr(added_suffix);
      if (prevmap) wr(std::string_view((const char*)prevmap, statbuf.st_size));

      assert(-1 != munmap(map, new_size));
      if (prevmap) assert(-1 != munmap(prevmap, statbuf.st_size));
      assert(-1 != close(fd));
      assert(-1 != close(prevfd));
      last_write_time(target, last_write_time(file));
      file = target;
    }
  }

  auto include_meta_dir = build_dir / "include_dirs";
  if (exists(include_meta_dir)) remove_all(include_meta_dir);
  create_directory(include_meta_dir);

  std::ofstream rsp_file(include_meta_dir / "args.rsp");

  {
    auto dir = include_meta_dir / "regular";
    assert(create_directory(dir));
    rsp_file << "-I " << dir << std::endl;
    for (auto&& file : files) {
      if (file.extension() != ".hpp") continue;
      if (file.filename() == "default.hpp") continue;
      auto target = dir / file.lexically_relative(copy_dir).parent_path() / file.stem();
      create_directories(target.parent_path());
      create_hard_link(file, target);
    }
  }

  {
    // default includes
    for (auto&& file : files) {
      if (file.filename() != "default.hpp") continue;
      auto lexfile = file.lexically_relative(copy_dir);
      auto topdir = include_meta_dir / std::to_string(std::ranges::distance(lexfile));
      auto target = topdir / lexfile.parent_path();
      if (create_directory(topdir)) rsp_file << "-I " << topdir << std::endl;
      create_directories(target.parent_path());
      create_hard_link(file, target);
    }
  }
}
