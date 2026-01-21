#include <ivl/build_system/parse_codebase>

// IVL add_compiler_flags("-g")
// IVL add_compiler_flags_tail("-lstdc++exp")

int main(int argc, char* argv[]) {
  assert(argc == 2);
  // std::filesystem::path file(argv[1]);
  // auto pp = *ivl::build_system::preprocess(file);
  // std::println("CONTENTS\n{}~CONTENTS", pp);
  // for (auto&& l : ivl::build_system::extract_ivl_directives(pp)) {
  //   std::println("from: {}", l.file.native());
  //   std::println("line: {}", l.pragma);
  //   for (auto el : ivl::build_system::parse_pragma_arg(l.pragma))
  //     std::println("- piece: {}", el);
  // }

  ivl::todo();

  try {
    std::filesystem::path dir(argv[1]);
    for (auto&& target : ivl::build_system::parse_ivl(dir)) {
      LOG(target.file, target.has_reg_variant, target.has_test_variant);
      for (auto&& compiler_flag : target.add_compiler_flags) LOG(compiler_flag);
      for (auto&& compiler_flag_tail : target.add_compiler_flags_tail) LOG(compiler_flag_tail);
      for (auto&& dependency : target.dependencies) LOG(dependency);
      for (auto&& test_dependency : target.test_dependencies) LOG(test_dependency);
    }
  } catch (const ivl::base_exception& e) {
    e.dump(stderr);
  }
}
