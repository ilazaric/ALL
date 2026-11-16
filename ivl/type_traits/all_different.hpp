#pragma once

#include <concepts>

// Are all types different?
namespace ivl {
template<typename...>
struct all_different_wrapper;
template<>
struct all_different_wrapper<> {
  inline static constexpr bool value = true;
};
template<typename Head, typename... Tail>
struct all_different_wrapper<Head, Tail...> {
  inline static constexpr bool value =
    (true && ... && !std::same_as<Head, Tail>) && all_different_wrapper<Tail...>::value;
};
template<typename... Ts>
concept all_different = all_different_wrapper<Ts...>::value;
} // namespace ivl
