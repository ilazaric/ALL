#pragma once

// Metaprogramming utilities.

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace ivl::meta {

// TODO: when we get universal template params, think how stuff can improve here
template <typename...>
struct type_list {};

template <typename T>
struct tag {
  using type = T;
};

template <typename T, typename... Ts>
concept same_as_one_of = (std::same_as<T, Ts> || ...);

// Are all types different?
template <typename...>
struct is_unique;
template <>
struct is_unique<> {
  inline static constexpr bool value = true;
};
template <typename Head, typename... Tail>
struct is_unique<Head, Tail...> {
  inline static constexpr bool value = (true && ... && !std::same_as<Head, Tail>) && is_unique<Tail...>::value;
};

template <typename... Ts>
inline constexpr bool is_unique_v = is_unique<Ts...>::value;

// TODO: add callable(index) variant
void repeat(size_t n, auto&& callable) {
  for (size_t i = 0; i < n; ++i) callable();
}

template <typename T>
struct construct_t {
  static constexpr T operator()(auto&&... args) { return T(std::forward<decltype(args)>(args)...); }
};
template <typename T>
inline constexpr construct_t<T> construct;

// don't put this on the heap
template <typename T>
union uninitialized {
  T data;
  uninitialized() {}
  ~uninitialized() {}
};

} // namespace ivl::meta
