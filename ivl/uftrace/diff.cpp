#include <ivl/command_line_argument_parsing/implicit_exposed>
#include <ivl/format>
#include <ivl/logger>
#include <ivl/ranges/split>
#include "compress"
#include "default"
#include <algorithm>
#include <filesystem>
#include <mdspan>
#include <ranges>
#include <set>

namespace uf = ivl::uftrace;

template<typename T>
std::vector<std::pair<size_t, size_t>> longest_common_subsequence(const std::vector<T>& l, const std::vector<T>& r) {
  std::vector<size_t> cache((l.size() + 1) * (r.size() + 1), 0);
  std::mdspan dp(cache.data(), l.size() + 1, r.size() + 1);
  for (size_t i = l.size() - 1; i + 1; --i)
    for (size_t j = r.size() - 1; j + 1; --j) {
      if (l[i] == r[j]) dp[i, j] = 1 + dp[i + 1, j + 1];
      else dp[i, j] = std::max(dp[i + 1, j], dp[i, j + 1]);
    }
  std::vector<std::pair<size_t, size_t>> ret;
  size_t i = 0, j = 0;
  while (i < l.size() && j < r.size()) {
    if (l[i] == r[j]) {
      ret.emplace_back(i++, j++);
      continue;
    }
    if (dp[i + 1, j] >= dp[i, j + 1]) ++i;
    else ++j;
  }
  return ret;
}

// void diff(uf::compressor& c, uf::compressor_handle l, uf::compressor_handle r, size_t indent = 0) {
//   if (l == r) return;
//   auto lcalls = c[l].calls;
//   auto rcalls = c[r].calls;
//   // looking for longest subsequence in both
// }

void fill(uf::compressor& c, uf::compressor_handle h, std::set<size_t>& out) {
  if (out.contains(h.index)) return;
  out.insert(h.index);
  for (auto el : c[h].calls) fill(c, el, out);
}

void print_missing(uf::compressor& c, uf::compressor_handle h, const std::set<size_t>& blacklist, size_t indent = 0) {
  if (blacklist.contains(h.index)) return;
  std::string_view name = c[h].name;
  if (c[h].calls.empty()) {
    ivl::fmt::println("{:s}{};", std::views::repeat(' ', indent), name);
    return;
  }
  ivl::fmt::println("{:s}{} {{", std::views::repeat(' ', indent), name);
  for (auto el : c[h].calls) print_missing(c, el, blacklist, indent + 2);
  if (name.ends_with("()")) name.remove_suffix(2);
  ivl::fmt::println("{:s}}} /* {} */", std::views::repeat(' ', indent), name);
}

void print(uf::compressor& c, uf::compressor_handle h, size_t indent = 0) {
  std::string_view name = c[h].name;
  if (c[h].calls.empty()) {
    ivl::fmt::println("{:s}{};", std::views::repeat(' ', indent), name);
    return;
  }
  ivl::fmt::println("{:s}{} {{", std::views::repeat(' ', indent), name);
  for (auto el : c[h].calls) print(c, el, indent + 2);
  if (name.ends_with("()")) name.remove_suffix(2);
  ivl::fmt::println("{:s}}} /* {} */", std::views::repeat(' ', indent), name);
}

uf::compressor_handle filter(uf::compressor& c, uf::compressor_handle h, auto&& pred) {
  uf::compressor_node r{.name = c[h].name};
  for (auto el : c[h].calls)
    if (pred(el, c[el])) r.calls.push_back(filter(c, el, pred));
  return c.insert(std::move(r));
}

void diff(uf::compressor& c, uf::compressor_handle l, uf::compressor_handle r) {
  ivl::fmt::println("printing diff ...");
  std::set<size_t> rs;
  fill(c, r, rs);
  l = filter(c, l, [&](auto h, auto&&) { return !rs.contains(h.index); });
  print(c, l);
  ivl::fmt::println("printing diff done\n");
}

size_t deep_size(uf::compressor& c, uf::compressor_handle h) {
  size_t ret = 1;
  for (auto el : c[h].calls) ret += deep_size(c, el);
  return ret;
}

int ivl_main(ivl::cmdline_parsing::implicit, const std::filesystem::path& lpath, const std::filesystem::path& rpath) {
  auto lroot = uf::parse_replay_file(lpath, implicit_flag("parse_describe"), implicit_flag("parse_verify"));
  auto rroot = uf::parse_replay_file(rpath, implicit_flag("parse_describe"), implicit_flag("parse_verify"));
  // diff(lroot, rroot);
  uf::compressor c;
  auto l = c.compress(lroot);
  auto r = c.compress(rroot);
  LOG(c.nodes.size());
  LOG(deep_size(c, l));
  LOG(deep_size(c, r));
  ivl::fmt::println();

  auto boring = implicit_value("boring", std::string_view());

  if (implicit_flag("find_boring")) {
    for (auto piece : ivl::views::split(boring, ",")) {
      ivl::fmt::println("looking for {:?} ...", piece);
      size_t cnt = 0;
      for (auto&& f : c.nodes)
        if (f.name.contains(piece)) {
          ++cnt;
          if (implicit_flag("find_boring_verbose")) ivl::fmt::println("  {}", f.name);
        }
      ivl::fmt::println("found {} names\n", cnt);
    }
    ivl::fmt::println();
  }

  if (implicit_flag("purge_boring")) {
    std::set<std::string> blacklist;
    for (auto piece : ivl::views::split(boring, ","))
      for (auto&& f : c.nodes)
        if (f.name.contains(piece)) blacklist.insert(f.name);
    ivl::fmt::println("purging {} names ...", blacklist.size());
    auto pred = [&](auto, const uf::compressor_node& f) { return !blacklist.contains(f.name); };
    l = filter(c, l, pred);
    r = filter(c, r, pred);
    LOG(deep_size(c, l));
    LOG(deep_size(c, r));
    ivl::fmt::println();
  }

  if (implicit_flag("diff")) diff(c, l, r);
  if (implicit_flag("diff_r")) diff(c, r, l);
  return 0;
}
