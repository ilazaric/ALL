#include <cassert>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <vector>

// TODO: check symlinks (want to ignore them)
// TODO: arg parsing, --help, --verbose-{x,y,z}

int main() {
  auto root = std::filesystem::canonical("/proc/self/exe");
  while (!exists(root / ".git")) {
    assert(root.has_parent_path());
    root = root.parent_path();
  }
  std::cerr << "repository root: " << root << std::endl;

  std::vector<std::filesystem::path> files;
  for (auto&& entry : std::filesystem::recursive_directory_iterator(root / "ivl")) {
    if (!entry.is_regular_file()) continue;
    auto&& p = entry.path();
    if (p.extension() == ".cpp" || p.extension() == ".hpp") files.emplace_back(p);
  }

  auto build_dir = root / "build";
  if (exists(build_dir)) {
    remove_all(build_dir);
    create_directory(build_dir);
  }

  auto copy_dir = build_dir / "source_copy";
  for (auto&& file : files) {
    auto target = copy_dir / file.lexically_relative(root);
    create_directories(target.parent_path());
    assert(copy_file(file, target));
    file = target;
  }

  auto include_meta_dir = build_dir / "include_dirs";

  {
    auto dir = include_meta_dir / "regular";
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
      auto target = include_meta_dir / std::to_string(std::ranges::distance(lexfile)) / lexfile.parent_path();
      create_directories(target.parent_path());
      create_hard_link(file, target);
    }
  }
}
