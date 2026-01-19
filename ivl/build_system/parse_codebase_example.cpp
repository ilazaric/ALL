#include <ivl/build_system/parse_codebase>

int main(int argc, char* argv[]) {
  assert(argc == 2);
  std::filesystem::path file(argv[1]);
  auto pp = ivl::build_system::preprocess(file);
  std::println("CONTENTS\n{}~CONTENTS", pp);
  for (auto&& l : ivl::build_system::extract_ivl_directives(pp))
    std::println("line: {}", l);
}
