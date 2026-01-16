#pragma once

namespace ivl::build_system {

struct target {
  std::vector<std::filesystem::path> inputs;
  std::vector<std::filesystem::path> outputs;
};

  void copy(const std::filesystem::path& in,
            const std::filesystem::path& out) {
    
  }

  void parse_ivl(const std::filesystem::path& src) {
    for (auto&& entry : std::filesystem::recursive_directory_iterator(src)) {
      if (!entry.is_regular_file()) continue;
      auto&& path = entry.path();
      assert(path.extension() == ".cpp" || path.extension() == ".hpp");
      
    }
  }

} // namespace ivl::build_system
