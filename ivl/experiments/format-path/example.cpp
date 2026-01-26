#include "formatter.hpp"
#include <print>
#include <filesystem>

int main() {
  std::filesystem::path p{"/usr/include"};
  std::println("{}", p);
}
