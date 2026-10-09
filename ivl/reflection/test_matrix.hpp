#pragma once

#include <meta>
#include <ranges>

namespace ivl {
template<typename T>
struct const_span {
  const T* data;
  size_t length;
  consteval const_span() : data(nullptr), length(0) {}
  consteval const_span(std::span<const T> s) : data(s.data()), length(s.size()) {}
  consteval const T* begin() const { return data; }
  consteval const T* end() const { return data + length; }
};

struct test_matrix {
  const_span<const_span<std::meta::info>> data;

  consteval test_matrix(auto&&... args) {
    std::vector<const_span<std::meta::info>> tmp;
    // TODO: types probably shouldnt be wrapped
    ((tmp.emplace_back(std::define_static_array(std::array{std::meta::reflect_constant(args)}))), ...);
    data = std::define_static_array(tmp);
  }

  consteval test_matrix(std::from_range_t, auto&& arg) {
    std::vector<const_span<std::meta::info>> tmp;
    // TODO: types probably shouldnt be wrapped
    for (auto&& el : arg) tmp.emplace_back(std::define_static_array(std::array{std::meta::reflect_constant(el)}));
    data = std::define_static_array(tmp);
  }

  friend consteval test_matrix operator*(test_matrix a, test_matrix b) {
    std::vector<const_span<std::meta::info>> tmp;
    for (auto ael : a.data)
      for (auto bel : b.data) tmp.emplace_back(std::define_static_array(std::views::concat(ael, bel)));
    test_matrix ret;
    ret.data = std::define_static_array(tmp);
    return ret;
  }

  friend consteval test_matrix operator+(test_matrix a, test_matrix b) {
    test_matrix ret;
    ret.data = std::define_static_array(std::views::concat(a.data, b.data));
    return ret;
  }
};

struct test_broadcast {
  std::meta::info temp;
  test_matrix matrix;
};
} // namespace ivl
