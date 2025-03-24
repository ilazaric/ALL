#include "phase1.hpp"
#include "phase2.hpp"
#include <ivl/logger>

int main(){
  auto src = ivl::cppp::phase2(ivl::cppp::phase1("input"));
  for (auto c : src) std::cout << (char)c;
}
