#pragma once

#include <ranges>
#include <string_view>

namespace ivl::ranges {
struct split_sentinel {};

struct split_iterator {
  using iterator_category = std::forward_iterator_tag;
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

  split_iterator operator++(int) {
    auto copy = *this;
    ++*this;
    return copy;
  }

  bool reached_end() const { return data.data() == nullptr; }

  bool operator==(split_sentinel) const { return reached_end(); }
  friend bool operator==(split_sentinel, const split_iterator& it) { return it.reached_end(); }
  bool operator==(const split_iterator&) const = default;
};

struct split_view : std::ranges::view_interface<split_view> {
  std::string_view data;
  std::string_view by;
  split_iterator begin() const { return split_iterator{data, by}; }
  split_sentinel end() const { return split_sentinel{}; }
};
} // namespace ivl::ranges

template<>
constexpr bool std::ranges::enable_borrowed_range<ivl::ranges::split_view> = true;

static_assert(std::ranges::forward_range<ivl::ranges::split_view>);
static_assert(std::ranges::borrowed_range<ivl::ranges::split_view>);
static_assert(std::ranges::view<ivl::ranges::split_view>);

namespace ivl::views {
auto split(std::string_view sv, std::string_view by) { return ranges::split_view{{}, sv, by}; }
} // namespace ivl::views
