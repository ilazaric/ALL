#pragma once

#include <tuple>
#include <utility>

namespace ivl::detail {
template<typename T, size_t N>
struct tuple_broadcast_t {
  T callable;
  template<size_t I>
  decltype(auto) get() const {
    return callable.template operator()<I>();
  }
};

template<typename T, typename... Ts>
struct tuple_broadcast_types_t {
  T callable;
  template<size_t I>
  decltype(auto) get() const {
    return callable.template operator()<Ts...[I]>();
  }
};

template<typename T, auto... Vs>
struct tuple_broadcast_values_t {
  T callable;
  template<size_t I>
  decltype(auto) get() const {
    return callable.template operator()<Vs...[I]>();
  }
};
} // namespace ivl::detail

template<typename T, size_t N>
struct std::tuple_size<ivl::detail::tuple_broadcast_t<T, N>> : std::integral_constant<size_t, N> {};

template<size_t I, typename T, size_t N>
struct std::tuple_element<I, ivl::detail::tuple_broadcast_t<T, N>> {
  using type = decltype(std::declval<ivl::detail::tuple_broadcast_t<T, N>>().template get<I>());
};

template<typename T, typename... Ts>
struct std::tuple_size<ivl::detail::tuple_broadcast_types_t<T, Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {
};

template<size_t I, typename T, typename... Ts>
struct std::tuple_element<I, ivl::detail::tuple_broadcast_types_t<T, Ts...>> {
  using type = decltype(std::declval<ivl::detail::tuple_broadcast_types_t<T, Ts...>>().template get<I>());
};

template<typename T, auto... Vs>
struct std::tuple_size<ivl::detail::tuple_broadcast_values_t<T, Vs...>>
    : std::integral_constant<size_t, sizeof...(Vs)> {};

template<size_t I, typename T, auto... Vs>
struct std::tuple_element<I, ivl::detail::tuple_broadcast_values_t<T, Vs...>> {
  using type = decltype(std::declval<ivl::detail::tuple_broadcast_values_t<T, Vs...>>().template get<I>());
};

namespace ivl {
template<size_t N, typename T>
auto tuple_broadcast(T&& callable) {
  return detail::tuple_broadcast_t<T&&, N>(static_cast<T&&>(callable));
}

template<typename... Ts, typename T>
auto tuple_broadcast_types(T&& callable) {
  return detail::tuple_broadcast_types_t<T&&, Ts...>(static_cast<T&&>(callable));
}

template<auto... Vs, typename T>
auto tuple_broadcast_values(T&& callable) {
  return detail::tuple_broadcast_values_t<T&&, Vs...>(static_cast<T&&>(callable));
}
} // namespace ivl
