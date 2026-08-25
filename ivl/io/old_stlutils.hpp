#pragma once

#include <cstddef>
#include <iostream>
#include <ranges>
#include <vector>

// a vector `vec` is serialized as:
// `vec.size()` <SPACE> `vec[0]` <SPACE> `vec[1]` <SPACE> ... <SPACE> `vec[vec.size()-1]`
namespace ivl::io::vector_size_elems {
template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec) {
  out << vec.size();
  for (auto& elem : vec) out << " " << elem;
  return out;
}

template<typename T>
std::istream& operator>>(std::istream& in, std::vector<T>& vec) {
  vec.clear();
  std::size_t len;
  in >> len;
  vec.reserve(len);
  T value;
  for (std::size_t i = 0; i < len; ++i) {
    in >> value;
    // `emplace_back` does same thing here
    vec.push_back(std::move(value));
  }
  return in;
}
} // namespace ivl::io::vector_size_elems

// a vector `vec` is serialized as:
// `vec[0]` <SPACE> `vec[1]` <SPACE> ... <SPACE> `vec[vec.size()-1]`
// size is assumed to be `vec.size()`
namespace ivl::io::vector_elems {
template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec) {
  if (!vec.empty()) out << vec.front();
  for (auto& elem : vec | std::views::drop(1)) out << " " << elem;
  return out;
}

template<typename T>
std::istream& operator>>(std::istream& in, std::vector<T>& vec) {
  for (auto& elem : vec) in >> elem;
  return in;
}
} // namespace ivl::io::vector_elems
