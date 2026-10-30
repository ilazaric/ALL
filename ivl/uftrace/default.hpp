#pragma once

#include <ivl/contracts>
#include <ivl/format>
#include <ivl/linux/utility>
#include <ivl/ranges/split>
#include <ivl/utility/pair_access>
#include <algorithm>
#include <iterator>
#include <map>
#include <string>
#include <string_view>
#include <vector>

// We deal in a specific uftrace replay format, as emitted by:
// `uftrace replay -d "$DATA" -f none --no-merge --no-event`
//
// Multi-threaded or multi-process is not supported.

namespace ivl::uftrace {
struct node {
  std::string name;
  std::vector<node> calls;

  size_t size() const {
    size_t acc = 1;
    for (auto&& el : calls) acc += el.size();
    return acc;
  }

  void occurrences_into(auto& out) const {
    ++out[name];
    for (auto&& el : calls) el.occurrences_into(out);
  }

  template<typename C>
  C occurrences() const {
    C ret;
    occurrences_into(ret);
    return ret;
  }

  template<template<typename, typename> typename C>
  auto occurrences() const {
    return occurrences<C<std::string, size_t>>();
  }

  auto occurrences() const { return occurrences<std::map>(); }

  void remove_if(auto&& cb) {
    auto slot = calls.begin();
    for (auto curr = calls.begin(); curr != calls.end(); ++curr) {
      if (cb(*curr)) continue;
      curr->remove_if(cb);
      if (curr != slot) *slot = std::move(*curr);
      ++slot;
    }
    calls.erase(slot, calls.end());
  }
};

template<typename It>
It serialize_into(It out, const node& f, size_t indent = 0);
// rootless can go through this
template<typename It>
It serialize_into(It out, const std::vector<node>& v, size_t indent = 0);

template<typename It>
It serialize_into(It out, const node& f, size_t indent) {
  out = std::fill_n(out, indent, ' ');
  out = std::ranges::copy(f.name, out).out;
  out = std::ranges::copy(std::string_view(" {\n"), out).out;
  out = serialize_into(out, f.calls, indent + 2);
  out = std::fill_n(out, indent, ' ');
  out = std::ranges::copy(std::string_view("} /* "), out).out;
  out = std::copy(f.name.begin(), f.name.end() - (f.name.ends_with("()") ? 2 : 0), out);
  out = std::ranges::copy(std::string_view(" */\n"), out).out;
  return out;
}

template<typename It>
It serialize_into(It out, const std::vector<node>& v, size_t indent) {
  for (auto&& el : v) out = serialize_into(out, el, indent);
  return out;
}

std::string serialize(const node& f, size_t indent = 0) {
  std::string ret;
  serialize_into(std::back_inserter(ret), f, indent);
  return ret;
}

// rootless can go through this
std::string serialize(const std::vector<node>& v, size_t indent = 0) {
  std::string ret;
  serialize_into(std::back_inserter(ret), v, indent);
  return ret;
}

void describe(const node& f, size_t n = 20) {
  fmt::println("node count: {}", f.size());
  auto occ = f.occurrences();
  fmt::println("number of distinct functions: {}", occ.size());
  fmt::println("{} most common functions:", n);
  std::vector<std::pair<std::string, size_t>> occ_vec(std::from_range, occ);
  std::ranges::sort(occ_vec, std::greater{}, second);
  for (auto&& [name, count] : std::views::take(occ_vec, n)) fmt::println("  {}: {}", name, count);
  fmt::println();
}

node parse_replay_text(std::string_view contents, bool desc = true, bool verify = false) {
  if (desc) fmt::println("describing uftrace parse from text of size: {}", contents.size());
  node root{.name = "~~~ ROOT ~~~"};
  std::vector<node*> stack{&root};
  for (auto&& line : views::split(contents, "\n")) {
    if (line.empty()) continue;
    if (line.ends_with("*/")) {
      stack.pop_back();
      continue;
    }
    contract_assert(line.ends_with(" {"));
    size_t indent = stack.size() * 2 - 2;
    stack.back()->calls.emplace_back(std::string(line.substr(indent, line.size() - 2 - indent)));
    stack.push_back(&stack.back()->calls.back());
  }
  contract_assert(stack.size() == 1);
  contract_assert(stack[0] == &root);
  if (desc) describe(root);
  if (verify) assert_text_equal(contents, serialize(root.calls));
  return root;
}

node parse_replay_file(const std::filesystem::path& p, bool desc = true, bool verify = false) {
  if (desc) fmt::println("describing uftrace parse from: {}", p);
  return parse_replay_text(linux::read_file(p), desc, verify);
}
} // namespace ivl::uftrace
