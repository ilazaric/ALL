#include <ivl/linux/utility>
#include <ivl/stl/string>
#include <filesystem>
#include <map>

std::vector<std::string> controllers(const std::filesystem::path& cgroup_dir) {
  return ivl::split_py(ivl::linux::read_file_slow(cgroup_dir / "cgroup.controllers"));
}

struct args {
  std::filesystem::path root = "/sys/fs/cgroup";
};

int ivl_main(const args& args) {
  auto root_controllers = controllers(args.root);
  std::map<std::string, size_t> controller_indices;
  for (size_t i = 0; i < root_controllers.size(); ++i) controller_indices[root_controllers[i]] = i;

  for (auto&& con : root_controllers) std::print("{} ", con);
  std::println("{}", args.root);

  for (auto&& entry : std::filesystem::recursive_directory_iterator(args.root)) {
    if (!entry.is_directory()) continue;

    auto con = controllers(entry.path());
    std::vector<bool> enabled(root_controllers.size(), false);
    for (auto&& el : con) enabled[controller_indices.at(el)] = true;
    for (size_t i = 0; i < root_controllers.size(); ++i)
      std::print("{: ^{}} ", enabled[i] ? '|' : ' ', root_controllers[i].size());
    auto rel = entry.path().lexically_relative(args.root);
    std::println(
      "{: <{}}{}", "", (std::ranges::distance(entry.path()) - std::ranges::distance(args.root)) * 2,
      entry.path().filename()
    );
  }
  return 0;
}
