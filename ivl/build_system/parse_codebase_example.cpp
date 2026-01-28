#include <ivl/build_system/parse_codebase>
#include <ivl/reflection/json>
#include <ivl/util>
#include <fstream>

// IVL add_compiler_flags("-g")
// IVL add_compiler_flags_tail("-lstdc++exp")

// struct big_manifest {
//   std::map<std::string, state> targets;
// };

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

  // g++ \
  -g \
  -DIVL_FILE="ivl/build_system/parse_codebase_example.cpp" \
  -include /home/ilazaric/repos/ALL/build/source_copy/ivl/build_system/parse_codebase_example.cpp \
  -lstdc++exp

  std::filesystem::path root = ivl::util::repo_root();
  auto build_dir = root / "build";

  ivl::process_config cxx_cfg;
  cxx_cfg.pathname = "/opt/GCC/bin/g++";
  cxx_cfg.argv = {
    cxx_cfg.pathname,
    "-xc++",
    "-Wl,-rpath=/opt/GCC/lib64",
    "-DIVL_LOCAL",
    "-O3",
    "-std=c++26",
    "-freflection",
    std::format("-ffile-prefix-map={}/=", root.native()),
    std::format("@{}/include_dirs/args.rsp", build_dir.native()),
  };
  cxx_cfg.envp = {
    {"LC_ALL", "C"},
    {"PATH", cxx_cfg.pathname.parent_path()},
  };

  // ivl::todo();

  try {
    std::filesystem::path dir(argv[1]);
    auto targets = ivl::build_system::parse_ivl(dir, cxx_cfg);
    for (auto&& target : targets) {
      LOG(target.file, target.has_reg_variant, target.has_test_variant);
      for (auto&& compiler_flag : target.add_compiler_flags) LOG(compiler_flag);
      for (auto&& compiler_flag_tail : target.add_compiler_flags_tail) LOG(compiler_flag_tail);
      for (auto&& dependency : target.dependencies) LOG(dependency);
      for (auto&& test_dependency : target.test_dependencies) LOG(test_dependency);
    }
    std::ofstream of("/home/ilazaric/repos/ALL/ivl/build_system/dump.json");
    of << ivl::to_json(targets).dump(2) << std::endl;
  } catch (const ivl::base_exception& e) {
    e.dump(stderr);
  }
}
