#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>

// TODO: move to ivl/util

namespace ivl::langs::tiny::meta {

  template <typename T>
  struct tag {
    using type = T;
  };

  template <typename...>
  struct tl {};

  template <typename...>
  struct tl_concat;
  template <>
  struct tl_concat<> {
    using type = tl<>;
  };
  template <typename... Ts>
  struct tl_concat<tl<Ts...>> {
    using type = tl<Ts...>;
  };
  template <typename... Ts, typename... Us, typename... Tail>
  struct tl_concat<tl<Ts...>, tl<Us...>, Tail...> {
    using type = tl_concat<tl<Ts..., Us...>, Tail...>::type;
  };

  template <typename>
  struct tl_length;
  template <typename... Ts>
  struct tl_length<tl<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
  };

  template <typename, typename>
  struct tl_contains;
  template <typename T, typename... Ts>
  struct tl_contains<T, tl<Ts...>> {
    static constexpr bool value = (false || ... || std::same_as<T, Ts>);
  };

  template <typename, typename>
  struct tl_find;
  template <typename T, typename... Ts>
  struct tl_find<T, tl<T, Ts...>> {
    static constexpr size_t value = 0;
  };
  template <typename T, typename R, typename... Ts>
  struct tl_find<T, tl<R, Ts...>> {
    static constexpr size_t value = 1 + tl_find<T, tl<Ts...>>::value;
  };

  template <typename>
  struct tl_is_unique;
  template <>
  struct tl_is_unique<tl<>> {
    static constexpr bool value = true;
  };
  template <typename T, typename... Ts>
  struct tl_is_unique<tl<T, Ts...>> {
    static constexpr bool value = (true && ... && !std::same_as<T, Ts>) && tl_is_unique<tl<Ts...>>::value;
  };

} // namespace ivl::langs::tiny::meta
