#include "passthrough"
#include <ivl/format>

int ivl_main(ivl::cmdline_parsing::passthrough args) {
  for (size_t i = 0; i < args.data.size(); ++i) ivl::fmt::println("{} -> {}", i, args.data[i]);
  return 0;
}
