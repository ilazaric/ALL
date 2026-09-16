#include <sys/mman.h>
#include <sys/stat.h>
#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string_view>
#include <unistd.h>
#include <vector>

// This program has to be buildable without any ivl headers accessible,
// because this program is the one that sets up the directory hierarchy of headers.

// TODO: check symlinks (want to ignore them)
// TODO: arg parsing, --help, --verbose-{x,y,z}
// UPDT: should probably go through ivl_main shenanigans
// UPDT: def shouldnt go through ivl_main , since that requires generated source copy

std::vector<std::filesystem::path> find_files(const std::filesystem::path& dir) {
  std::vector<std::filesystem::path> ret;
  for (auto it = std::filesystem::recursive_directory_iterator(dir);
       it != std::filesystem::recursive_directory_iterator(); ++it) {
    if (it->path().filename() == ".git") {
      it.disable_recursion_pending();
      continue;
    }
    if (it->path().filename() == ".gitmodules") continue;
    if (it->is_regular_file()) ret.push_back(it->path());
  }
  return ret;
}

inline bool is_cpp_file(const std::filesystem::path& p) {
  return p.extension() == ".cpp" || p.extension() == ".hpp" || p.extension() == ".c" || p.extension() == ".h" ||
         p.extension() == ".cc";
}

std::vector<std::filesystem::path> find_sources(const std::filesystem::path& dir) {
  std::vector<std::filesystem::path> ret;
  for (auto&& p : find_files(dir)) {
    if (is_cpp_file(p)) ret.push_back(p);
  }
  return ret;
}

void sync_file(
  const std::filesystem::path& file, const std::filesystem::path& target, std::string_view added_prefix = {}
) {
  const auto lwt_start = last_write_time(file);
  if (exists(target)) return;
  create_directories(target.parent_path());

  auto prevfd = open(file.native().c_str(), O_RDONLY, 0);
  assert(prevfd != -1);
  struct stat statbuf;
  assert(-1 != fstat(prevfd, &statbuf));
  size_t new_size = statbuf.st_size + added_prefix.size();
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
  auto map = new_size ? mmap(nullptr, new_size, PROT_WRITE, MAP_SHARED, fd, 0) : nullptr;
  if (map == MAP_FAILED) {
    auto e = errno;
    std::cout << "file: " << file << std::endl;
    std::cout << "target: " << target << std::endl;
    std::cout << "err: " << e << std::endl;
    assert(false);
  }
  char* ptr = (char*)map;
  auto wr = [&](std::string_view sv) {
    if (sv.empty()) return;
    memcpy(ptr, sv.data(), sv.size());
    ptr += sv.size();
  };
  wr(added_prefix);
  if (prevmap) wr(std::string_view((const char*)prevmap, statbuf.st_size));

  if (map) assert(-1 != munmap(map, new_size));
  if (prevmap) assert(-1 != munmap(prevmap, statbuf.st_size));
  assert(-1 != close(fd));
  assert(-1 != close(prevfd));
  const auto lwt_end = last_write_time(file);
  assert(lwt_start == lwt_end);
  last_write_time(target, lwt_end);
}

void purge_outdated(const std::filesystem::path& indir, const std::filesystem::path& outdir) {
  if (!exists(outdir)) return;
  for (auto&& existing : find_files(outdir)) {
    auto original = indir / existing.lexically_relative(outdir);
    if (!exists(original) || last_write_time(original) != last_write_time(existing)) remove(existing);
  }
}

void sync_dir(const std::filesystem::path& indir, const std::filesystem::path& outdir) {
  assert(exists(indir));
  create_directories(outdir);
  for (auto&& file : find_files(indir)) {
    auto target = outdir / file.lexically_relative(indir);
    if (is_cpp_file(file)) sync_file(file, target, "#line 1 \"" + file.native() + "\"\n");
    else sync_file(file, target);
  }
}

void sync_sources(const std::filesystem::path& indir, const std::filesystem::path& outdir) {
  assert(exists(indir));
  create_directories(outdir);
  for (auto&& file : find_sources(indir)) {
    auto target = outdir / file.lexically_relative(indir);
    sync_file(file, target, "#line 1 \"" + file.native() + "\"\n");
  }
}

bool sync_file_if(const std::filesystem::path& in, const std::filesystem::path& out) {
  if (!exists(in)) return false;
  sync_file(in, out);
  return true;
}

bool sync_dir_if(const std::filesystem::path& indir, const std::filesystem::path& outdir) {
  if (!exists(indir)) return false;
  sync_dir(indir, outdir);
  return true;
}

bool sync_sources_if(const std::filesystem::path& indir, const std::filesystem::path& outdir) {
  if (!exists(indir)) return false;
  sync_sources(indir, outdir);
  return true;
}

int main() {
  auto root = std::filesystem::canonical("/proc/self/exe");
  while (!exists(root / ".git")) {
    assert(root.has_parent_path());
    root = root.parent_path();
  }
  std::cerr << "repository root: " << root << std::endl;

  auto build_dir = root / "build";
  auto copy_dir = build_dir / "source_copy";
  sync_sources(root / "ivl", copy_dir / "ivl");

  auto include_meta_dir = build_dir / "include_dirs";
  if (exists(include_meta_dir)) remove_all(include_meta_dir);
  create_directory(include_meta_dir);
  std::ofstream rsp_file(include_meta_dir / "args.rsp");

  auto sync_submodule = [&](std::string_view m, std::string_view inc = {}) {
    sync_dir(root / "submodules" / m, build_dir / "submodule_source_copy" / m);
    if (inc.empty()) return;
    auto p = build_dir / "submodule_source_copy" / m / inc;
    if (exists(p)) rsp_file << "-I " << p << std::endl;
  };
  sync_submodule("nlohmann-json", "include");
  sync_submodule("raylib");
  sync_submodule("pugixml");
  sync_submodule("fmt", "include");
  {
    auto inc = build_dir / "submodule_include";
    if (exists(inc)) remove_all(inc);
    create_directory(inc);
    create_directory_symlink("../submodule_source_copy/raylib/src", inc / "raylib");
    create_directory_symlink("../submodule_source_copy/pugixml/src", inc / "pugixml");
    rsp_file << "-I " << inc << std::endl;
  }

  auto files = find_sources(copy_dir / "ivl");

  {
    auto dir = include_meta_dir / "regular";
    assert(create_directory(dir));
    rsp_file << "-I " << dir << std::endl;
    for (auto&& file : files) {
      auto target = dir / file.lexically_relative(copy_dir).parent_path() /
                    (file.extension() == ".hpp" ? file.stem() : file.filename());
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
      // create_hard_link(file, target);
      std::ofstream(target) << "#include <" << lexfile.parent_path().native() << "/default>\n";
      last_write_time(target, last_write_time(file));
    }
  }
}
