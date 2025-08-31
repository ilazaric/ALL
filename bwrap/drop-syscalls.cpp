#include "syscall-mappings.hpp"
#include <cassert>
#include <cstring>
#include <seccomp.h>
#include <unistd.h>
// #include <iostream>

int main(int argc, char** argv) {
  assert(argv[0] != nullptr);
  auto            args = argv + 1;
  scmp_filter_ctx ctx  = seccomp_init(SCMP_ACT_ALLOW);
  if (ctx == NULL) {
    perror("seccomp_init");
    return 1;
  }
  for (; strcmp(*args, "--"); ++args) {
    // std::cerr << ".";
    assert(*args != nullptr);
    if (0 > seccomp_rule_add(ctx, SCMP_ACT_KILL, syscall2number.at(std::string(*args)), 0)) {
      perror("seccomp_rule_add");
      return 1;
    }
  }
  if (0 > seccomp_load(ctx)) {
    perror("seccomp_load");
    return 1;
  }
  ++args;
  assert(*args != nullptr);
  execv(*args, args);
}
