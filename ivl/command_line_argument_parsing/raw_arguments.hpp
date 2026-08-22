#pragma once

#include <span>

namespace ivl::cmdline_parsing {
// TODO: turn into struct, give string_view back
using raw_arguments = std::span<const char*>;
} // namespace ivl::cmdline_parsing
