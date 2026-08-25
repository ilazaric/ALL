#include <ivl/logger>
#include "phase1"
#include "phase2"

int main() {
  auto src = ivl::cppp::phase2(ivl::cppp::phase1("input"));
  for (auto c : src)
    std::cout << (char)c;
}
