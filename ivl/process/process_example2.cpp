#include <ivl/process>
#include <filesystem>
#include <ivl/format>

int main() try {
  ivl::process_function ls{.pathname = "/usr/bin/ls"};
  std::filesystem::path dir = "/sys";
  ivl::fmt::println("executing `ls {}`:\n{}", dir, ls(dir));
} catch (const ivl::base_exception& e) {
  e.dump();
}
