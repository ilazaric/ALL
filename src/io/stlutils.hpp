#pragma once

#include <iostream>
#include <ranges>
#include <vector>
#include <cstddef>
#include <type_traits>
#include <valarray>

// god forgive me, need this for ADL
namespace std {
  
  template<typename A, typename B>
  std::ostream& operator<<(std::ostream& out, const std::pair<A, B>& p){
    return out << p.first << " " << p.second;
  }

  template<typename A, typename B>
  std::istream& operator>>(std::istream& in, std::pair<A, B>& p){
    return in >> p.first >> p.second;
  }
  
  // size elem0 elem1 ... elemN-1
  template<typename T>
  std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec){
    out << vec.size();
    for (auto& elem : vec)
      out << " " << elem;
    return out;
  }

  template<typename T>
  std::istream& operator>>(std::istream& in, std::vector<T>& vec){
    vec.clear();
    std::size_t len;
    in >> len;
    vec.reserve(len);
    T value;
    for (std::size_t i = 0; i < len; ++i){
      in >> value;
      // `emplace_back` does same thing here
      vec.push_back(std::move(value));
    }
    return in;
  }
  
  // size elem0 elem1 ... elemN-1
  template<typename T>
  std::ostream& operator<<(std::ostream& out, const std::valarray<T>& vec){
    out << vec.size();
    for (auto& elem : vec)
      out << " " << elem;
    return out;
  }

  template<typename T>
  std::istream& operator>>(std::istream& in, std::valarray<T>& vec){
    std::size_t len;
    in >> len;
    vec = std::valarray<T>(len);
    for (std::size_t i = 0; i < len; ++i)
      in >> vec[i];
    return in;
  }

} // namespace std

namespace ivl::io {
  
  // this is useful if we want to read/write just the elements of something, no size
  template<typename T>
  struct Elems {
    T t;
  };

  template<typename T>
  Elems(T&&) -> Elems<T&&>;

  template<typename T>
  std::ostream& operator<<(std::ostream& out, const Elems<T>& elems){
    if (!elems.t.empty())
      out << *(elems.t.begin());
    for (const auto& elem : elems.t | std::views::drop(1))
      out << " " << elem;
    return out;
  }

  template<typename T>
  std::istream& operator>>(std::istream& in, Elems<T>&& elems){
    for (auto& elem : elems.t)
      in >> elem;
    return in;
  }

} // namespace ivl::io
