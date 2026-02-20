#include <ivl/process>
#include <filesystem>
#include <print>

int main() try {
  ivl::process_function ls{.pathname = "/usr/bin/ls"};
  std::filesystem::path dir = "/sys";
  std::println("executing `ls {}`:\n{}", dir, ls(dir));
} catch (const ivl::base_exception& e) {
  e.dump();
}
