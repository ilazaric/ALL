#include "formatter.hpp"
#include <cassert>
#include <filesystem>
#include <print>
#include <string>
#include <vector>

void paper_examples() {
  assert(std::format("{}", std::filesystem::path{"/usr/bin"}) == "/usr/bin");
  assert(std::format("{}", std::filesystem::path{"multi\nline"}) == "multi\nline");
  assert(std::format("{:?}", std::filesystem::path{"multi\nline"}) == "\"multi\\nline\"");
}

int main() {
  paper_examples();

  std::filesystem::path p{"/usr/include"};
  std::println("{}", p);
  std::vector<std::filesystem::path> vec{"/tmp", "/dev/shm", "/", "/usr/include", "../../.."};
  std::println("{}", vec);
}
