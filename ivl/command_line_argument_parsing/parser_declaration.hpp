#pragma once

#include "raw_arguments"
#include <optional>
#include <string_view>

namespace ivl::cmdline_parsing {
template<typename>
struct parser;

template<typename T>
concept parseable = requires(T& a, std::optional<std::string_view> b, raw_arguments c) { parser<T>{}.parse(a, b, c); };
} // namespace ivl::cmdline_parsing
