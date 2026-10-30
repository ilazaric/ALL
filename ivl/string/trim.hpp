#pragma once

#include <cctype>
#include <string>
#include <string_view>

// TODO: implement the generic basic_string{,_view} templates
// UPDT: actually, punt on this, first think about ufcs-esque extensions
// UPDT: using gcc mainly, so probably no ufcs-ish extensions

namespace ivl::string {
std::string_view trim_prefix_view(std::string_view sv) {
  while (!sv.empty() && std::isspace(sv.front())) sv.remove_prefix(1);
  return sv;
}

std::string_view trim_suffix_view(std::string_view sv) {
  while (!sv.empty() && std::isspace(sv.back())) sv.remove_suffix(1);
  return sv;
}

std::string_view trim_view(std::string_view sv) { return trim_prefix_view(trim_suffix_view(sv)); }

std::string trim_prefix(std::string_view sv) { return std::string(trim_prefix_view(sv)); }
std::string trim_suffix(std::string_view sv) { return std::string(trim_suffix_view(sv)); }
std::string trim(std::string_view sv) { return std::string(trim_view(sv)); }
} // namespace ivl::string
