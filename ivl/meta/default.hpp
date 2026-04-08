#pragma once

// Metaprogramming utilities.

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace ivl::meta {

// TODO: when we get universal template params, think how stuff can improve here
template<typename...>
struct type_list {};

template<typename T>
struct tag {
  using type = T;
};

template<typename T, typename... Ts>
concept same_as_one_of = (std::same_as<T, Ts> || ...);

// Are all types different?
template<typename...>
struct is_unique_impl;
template<>
struct is_unique_impl<> {
  inline static constexpr bool value = true;
};
template<typename Head, typename... Tail>
struct is_unique_impl<Head, Tail...> {
  inline static constexpr bool value = (true && ... && !std::same_as<Head, Tail>) && is_unique_impl<Tail...>::value;
};

template<typename... Ts>
concept is_unique = is_unique_impl<Ts...>::value;

template<typename T>
concept is_trivially_destructible = std::is_trivially_destructible_v<T>;

// TODO: add callable(index) variant
// UPDT: probably actually just remove this
void repeat(size_t n, auto&& callable) {
  for (size_t i = 0; i < n; ++i) callable();
}

template<typename T>
struct construct_t {
  static constexpr T operator()(auto&&... args) { return T(std::forward<decltype(args)>(args)...); }
};
template<typename T>
inline constexpr construct_t<T> construct;

// don't put this on the heap
template<typename T>
union uninitialized {
  T data;
  uninitialized() {}
  ~uninitialized() {}
};

} // namespace ivl::meta
