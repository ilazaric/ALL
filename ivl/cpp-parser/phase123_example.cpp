#include <ivl/cpp-parser/pp_tokens>
#include <ivl/cpp-parser/spliced_cxx_file>
#include <ivl/linux/utils>
#include <ivl/logger>
#include <cassert>
#include <filesystem>

int main(int argc, char* argv[]) {
  assert(argc == 2);

  ivl::spliced_cxx_file file(ivl::linux::read_file(argv[1]));
  // LOG("\n" + file.post_splicing_contents);

  std::vector<std::filesystem::path> search_list{
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/../../../../include/c++/16.0.0",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/../../../../include/c++/16.0.0/x86_64-pc-linux-gnu",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/../../../../include/c++/16.0.0/backward",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/include",
    "/usr/local/include",
    "/opt/GCC-REFL/include",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/include-fixed",
    "/usr/include/x86_64-linux-gnu",
    "/usr/include",
  };

  auto state = file.parsing_start();
  auto tokens = ivl::top_level_parse(state);
  assert(state.empty());

  for (auto&& token : tokens) std::cout << ivl::reserialize(token);
}
