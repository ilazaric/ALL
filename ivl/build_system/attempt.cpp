// #pragma once

#include <ivl/crypto/blake3>
#include <ivl/logger>
#include <ivl/process>
#include <cassert>
#include <filesystem>
#include <ranges>
#include <vector>

// In case a file mtime was changed, we compare BLAKE3 hash.
// If the hash hasn't changed, we don't regenerate downstream files.
// Default constructed represents "missing file" state.
struct content_identity {
  bool exists = false;
  int64_t mtime = 0;
  ivl::crypto::blake3::input_chain_value blake3_hash{};
};

// Returns true if the file content was changed.
bool compare_and_update(const std::filesystem::path& p, content_identity& id) {
  auto fd = ivl::linux::raw_syscalls::open(p.native().c_str(), O_RDONLY, (umode_t)O_CLOEXEC);

  // "file doesn't exist" case
  if (fd == -ENOENT) {
    if (!id.exists) return false;
    id.exists = false;
    id.mtime = 0;
    id.blake3_hash = {};
    return true;
  }

  struct stat statbuf;
  auto fstat_ret = ivl::linux::raw_syscalls::fstat(fd, &statbuf);
  assert(fstat_ret == 0);
  int64_t mtime = statbuf.st_mtim.tv_sec * 1'000'000'000LL + statbuf.st_mtim.tv_nsec;

  if (id.exists && id.mtime == mtime) return false;

  auto mmap_ret = ivl::linux::raw_syscalls::mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(mmap_ret > 0);
  auto blake3_hash =
    ivl::crypto::blake3::hash(std::string_view(reinterpret_cast<const char*>(mmap_ret), statbuf.st_size));
  auto munmap_ret = ivl::linux::raw_syscalls::munmap(mmap_ret, statbuf.st_size);
  assert(munmap_ret == 0);

  bool ret = (id.exists != true) || (id.blake3_hash != blake3_hash);
  id.exists = true;
  id.mtime = mtime;
  id.blake3_hash = blake3_hash;
  return ret;
}

struct job {
  std::vector<std::filesystem::path> input_files;
  std::vector<std::filesystem::path> output_files;
  size_t memory_limit;
  size_t cpu_limit_percent;
  // TODO: more?
  ivl::process_config pc;
};

struct graph {
  std::vector<job> jobs;
  std::unordered_map<std::filesystem::path, content_identity> identities;
  std::unordered_map<std::filesystem::path, size_t> generators;
};

int main(int argc, const char* argv[]) {
  std::vector<std::filesystem::path> arg_targets(argv + 1, argv + argc);
  auto wd = std::filesystem::current_path();
  LOG(wd);
  auto root = wd;
  while (!exists(root / ".git")) {
    assert(root.has_parent_path());
    root = root.parent_path();
  }
  LOG(root);

  std::vector<std::filesystem::path> files;
  for (auto&& entry : std::filesystem::recursive_directory_iterator(root / "ivl")) {
    if (!entry.is_regular_file()) continue;
    auto&& p = entry.path();
    if (p.extension() == ".cpp" || p.extension() == ".hpp") files.emplace_back(p);
  }
  // for (auto&& file : files) LOG(file);

  auto build_dir = root / "build";
  if (exists(build_dir)) {
    remove_all(build_dir);
    create_directory(build_dir);
  }

  auto copy_dir = build_dir / "source_copy";
  for (auto&& file : files) {
    auto target = copy_dir / file.lexically_relative(root);
    create_directories(target.parent_path());
    assert(copy_file(file, target, std::filesystem::copy_options::overwrite_existing));
    file = target;
  }

  auto include_meta_dir = build_dir / "include_dirs";

  auto write_string_to_file = [](const std::filesystem::path& path, std::string_view data) {
    auto fd = ivl::linux::raw_syscalls::creat(path.native().c_str(), 0644);
    if (fd < 0) {
      std::cerr << "failed to open file\n";
      LOG(path, fd);
      exit(1);
    }

    while (!data.empty()) {
      auto cnt = ivl::linux::raw_syscalls::write(fd, data.data(), data.size());
      if (cnt < 0) {
        std::cerr << "failed to write to file\n";
        LOG(path);
        exit(1);
      }
      assert(cnt <= data.size());
      data.remove_prefix(cnt);
    }

    auto ret = ivl::linux::raw_syscalls::close(fd);
    if (ret < 0) {
      std::cerr << "failed to close file\n";
      LOG(path, ret);
      exit(1);
    }
  };

  {
    auto dir = include_meta_dir / "regular";
    for (auto&& file : files) {
      if (file.extension() != ".hpp") continue;
      auto target = dir / file.lexically_relative(copy_dir).parent_path() / file.stem();
      create_directories(target.parent_path());
      write_string_to_file(
        target, "// GENERATED FILE, DO NOT EDIT\n"
                "#include \"" +
                  (file.parent_path().lexically_relative(target.parent_path()) / file.filename()).native() + "\"\n"
      );
    }
  }

  {
    // default includes
    for (auto&& file : files) {
      if (file.filename() != "default.hpp") continue;
      auto lexfile = file.lexically_relative(copy_dir);
      auto target = include_meta_dir / std::to_string(std::ranges::distance(lexfile)) / lexfile.parent_path();
      create_directories(target.parent_path());
      write_string_to_file(
        target, "// GENERATED FILE, DO NOT EDIT\n"
                "#include \"" +
                  (file.parent_path().lexically_relative(target.parent_path()) / file.filename()).native() + "\"\n"
      );
    }
  }

  std::vector<std::filesystem::path> path_targets;
  for (auto&& arg_target : arg_targets)
    path_targets.emplace_back(arg_target.is_relative() ? wd / arg_target : root / "ivl" / arg_target.relative_path());
  // for (auto&& target : path_targets) LOG(target);
  for (auto&& target : path_targets) target = target.lexically_normal();

  std::vector<std::filesystem::path> globbed_targets;
  {
    bool failed = false;
    for (auto&& target : path_targets) {
      if (exists(target)) {
        if (is_directory(target)) {
          for (auto&& entry : std::filesystem::recursive_directory_iterator(target))
            if (is_regular_file(entry) && entry.path().extension() == ".cpp") globbed_targets.push_back(entry.path());
        } else globbed_targets.push_back(target);
        continue;
      }
      auto cpy = target;
      cpy.replace_extension("cpp");
      if (exists(cpy)) {
        globbed_targets.emplace_back(std::move(cpy));
        continue;
      }

      auto&& arg_target = arg_targets[&target - path_targets.data()];
      std::cerr << "ERROR: bad target: " << arg_target << std::endl;
      std::cerr << "     : corresponding path does not exist" << std::endl;
      std::cerr << "     : path: " << target << std::endl;
      failed = true;
    }

    if (failed) return 1;
  }

  for (auto&& target : globbed_targets) LOG(target);

  

  // std::vector<std::filesystem::path> files;
  // for (auto&& target : targets) {

  // }
}
