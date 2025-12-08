#include <ivl/crypto/blake3>
#include <fstream>
#include <iostream>
#include <print>
#include <sstream>

std::string consume(auto&& in) {
  std::stringstream buffer;
  buffer << in.rdbuf();
  return std::move(buffer).str();
}

int main(int argc, char** argv) {
  auto contents = argc == 2 ? consume(std::ifstream(argv[1])) : consume(std::cin);
  auto hash = ivl::crypto::blake3::hash(contents);
  std::string_view bla((char*)&hash, (char*)(&hash + 1));
  for (auto c : bla) std::print("{:02x}", c);
  std::println("  {}", argc == 2 ? argv[1] : "-");
}
