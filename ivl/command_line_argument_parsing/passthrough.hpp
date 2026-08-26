#pragma once

#include <span>

namespace ivl::cmdline_parsing {
struct passthrough {
  std::span<const char*> data;
};
} // namespace ivl::cmdline_parsing
