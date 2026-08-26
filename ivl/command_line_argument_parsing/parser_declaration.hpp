#pragma once

#include "raw_arguments"

namespace ivl::cmdline_parsing {
template<typename>
struct parser;

template<typename T>
concept parseable = requires(T& a, raw_arguments b) { parser<T>{}.parse(a, b); };
} // namespace ivl::cmdline_parsing
