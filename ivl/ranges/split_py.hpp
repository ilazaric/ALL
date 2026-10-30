#pragma once

#include <ivl/string/trim>
#include <ranges>

namespace ivl::ranges {
struct split_py_sentinel {};

struct split_py_iterator {
  using iterator_category = std::forward_iterator_tag;
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

  split_py_iterator operator++(int) {
    auto copy = *this;
    ++*this;
    return copy;
  }

  bool operator==(split_py_sentinel) const { return data.empty(); }
  friend bool operator==(split_py_sentinel, const split_py_iterator& it) { return it.data.empty(); }
  bool operator==(const split_py_iterator&) const = default;
};

struct split_py_view : std::ranges::view_interface<split_py_view> {
  std::string_view data;
  split_py_iterator begin() const { return split_py_iterator{data}; }
  split_py_sentinel end() const { return split_py_sentinel{}; }
};
} // namespace ivl::ranges

static_assert(std::ranges::forward_range<ivl::ranges::split_py_view>);
static_assert(std::ranges::borrowed_range<ivl::ranges::split_py_view>);
static_assert(std::ranges::view<ivl::ranges::split_py_view>);

namespace ivl::views {
auto split_py(std::string_view sv) { return ranges::split_py_view{{}, sv}; }
} // namespace ivl::views
