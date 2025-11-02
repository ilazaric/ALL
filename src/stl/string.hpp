#pragma once

#include <ivl/meta>
#include <cctype>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace ivl {

  // TODO: implement the generic basic_string{,_view} templates
  // actually, punt on this, first think about ufcs-esque extensions

  // TODO: move to ivl/string

  std::string_view trim_prefix_view(std::string_view sv) {
    while (!sv.empty() && std::isspace(sv.front()))
      sv.remove_prefix(1);
    return sv;
  }

  std::string_view trim_suffix_view(std::string_view sv) {
    while (!sv.empty() && std::isspace(sv.back()))
      sv.remove_suffix(1);
    return sv;
  }

  std::string_view trim_view(std::string_view sv) { return trim_prefix_view(trim_suffix_view(sv)); }

  std::string trim_prefix(std::string_view sv) { return std::string(trim_prefix_view(sv)); }
  std::string trim_suffix(std::string_view sv) { return std::string(trim_suffix_view(sv)); }
  std::string trim(std::string_view sv) { return std::string(trim_view(sv)); }

  struct split_py_sentinel {};

  struct split_py_iterator {
    using iterator_category = std::input_iterator_tag;
    using value_type        = std::string_view;
    using difference_type   = ptrdiff_t;
    using pointer           = const std::string_view*;
    using reference         = const std::string_view&;

    std::string_view data{};
    size_t           cursor{};

    explicit split_py_iterator(std::string_view sv) : data(sv), cursor(0) { ++*this; }
    split_py_iterator() = default;

    std::string_view operator*() const { return data.substr(0, cursor); }

    split_py_iterator& operator++() {
      data = trim_prefix_view(data.substr(cursor));
      for (cursor = 0; cursor < data.size() && !std::isspace(data[cursor]); ++cursor)
        ;
      return *this;
    }

    void operator++(int) { ++*this; }

    bool        operator==(split_py_sentinel) const { return data.empty(); }
    bool        operator!=(split_py_sentinel) const { return !data.empty(); }
    friend bool operator==(split_py_sentinel, const split_py_iterator& it) { return it.data.empty(); }
    friend bool operator!=(split_py_sentinel, const split_py_iterator& it) { return !it.data.empty(); }
  };

  struct split_py_range_t {
    std::string_view  data;
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

} // namespace ivl
