#include "passthrough"
#include <print>

int ivl_main(ivl::cmdline_parsing::passthrough args) {
  for (size_t i = 0; i < args.data.size(); ++i) std::println("{} -> {}", i, args.data[i]);
  return 0;
}
