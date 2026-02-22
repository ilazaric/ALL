#pragma once

#include <ivl/meta>
#include <cctype>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace ivl {

// TODO: implement the generic basic_string{,_view} templates
// UPDT: actually, punt on this, first think about ufcs-esque extensions
// UPDT: using gcc mainly, so probably no ufcs-ish extensions

// TODO: move to ivl/string

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

struct split_py_sentinel {};

struct split_py_iterator {
  using iterator_category = std::input_iterator_tag;
  using value_type = std::string_view;
  using difference_type = ptrdiff_t;
  using pointer = const std::string_view*;
  using reference = const std::string_view&;

  std::string_view data{};
  size_t cursor{};

  explicit split_py_iterator(std::string_view sv) : data(sv), cursor(0) { ++*this; }
  split_py_iterator() = default;

  std::string_view operator*() const { return data.substr(0, cursor); }

  split_py_iterator& operator++() {
    data = trim_prefix_view(data.substr(cursor));
    for (cursor = 0; cursor < data.size() && !std::isspace(data[cursor]); ++cursor);
    return *this;
  }

  void operator++(int) { ++*this; }

  bool operator==(split_py_sentinel) const { return data.empty(); }
  bool operator!=(split_py_sentinel) const { return !data.empty(); }
  friend bool operator==(split_py_sentinel, const split_py_iterator& it) { return it.data.empty(); }
  friend bool operator!=(split_py_sentinel, const split_py_iterator& it) { return !it.data.empty(); }
};

struct split_py_range_t {
  std::string_view data;
  split_py_iterator begin() const { return split_py_iterator{data}; }
  split_py_sentinel end() const { return split_py_sentinel{}; }
};

auto split_py_range(std::string_view sv) { return split_py_range_t{sv}; }

std::vector<std::string> split_py(std::string_view sv) {
  return split_py_range(sv) | std::views::transform(meta::construct<std::string>) | std::ranges::to<std::vector>();
}

std::vector<std::string_view> split_py_view(std::string_view sv) {
  return split_py_range(sv) | std::ranges::to<std::vector>();
}

struct split_sentinel {};

struct split_iterator {
  using iterator_category = std::input_iterator_tag;
  using value_type = std::string_view;
  using difference_type = ptrdiff_t;
  using pointer = const std::string_view*;
  using reference = const std::string_view&;

  std::string_view data{};
  std::string_view by{};
  size_t cursor{};

  explicit split_iterator(std::string_view sv, std::string_view by) : data(sv), by(by), cursor(0) {
    cursor = data.find(by);
  }
  split_iterator() = default;

  std::string_view operator*() const { return data.substr(0, cursor); }

  split_iterator& operator++() {
    if (cursor == std::string_view::npos) [[unlikely]] {
      data = {};
      cursor = 0;
      return *this;
    }
    data.remove_prefix(cursor);
    data.remove_prefix(by.size());
    cursor = data.find(by);
    return *this;
  }

  void operator++(int) { ++*this; }

  bool reached_end() const { return data.data() == nullptr; }

  bool operator==(split_sentinel) const { return reached_end(); }
  friend bool operator==(split_sentinel, const split_iterator& it) { return it.reached_end(); }
};

struct split_range_t {
  std::string_view data;
  std::string_view by;
  split_iterator begin() const { return split_iterator{data, by}; }
  split_sentinel end() const { return split_sentinel{}; }
};

auto split_range(std::string_view sv, std::string_view by) { return split_range_t{sv, by}; }

std::vector<std::string> split(std::string_view sv, std::string_view by) {
  return split_range(sv, by) | std::views::transform(meta::construct<std::string>) | std::ranges::to<std::vector>();
}

std::vector<std::string_view> split_view(std::string_view sv, std::string_view by) {
  return split_range(sv, by) | std::ranges::to<std::vector>();
}

} // namespace ivl
