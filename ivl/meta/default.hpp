#pragma once

// Metaprogramming utilities.

// TODO: <ivl/meta> should relate to <meta>, as in reflection
// ....: either kill this or move it somewhere

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace ivl::meta {
template<typename T>
struct tag {
  using type = T;
};

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

template<typename T, typename U>
concept is_unqualified = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
} // namespace ivl::meta
