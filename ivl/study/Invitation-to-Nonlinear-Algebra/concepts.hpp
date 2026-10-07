#pragma once

#include <concepts>

template<typename K>
K identity() {
  if constexpr (requires { { K::identity() } -> std::same_as<K>; }) {
    return K::identity();
  } else {
    return K(1);
  }
}

template<typename K>
concept ring = requires {
  { K{} };
  requires requires(K m, const K c) {
    { c + c } ; // -> std::same_as<K>;
    { m += c } ; // -> std::same_as<K&>;
    { c - c } ; // -> std::same_as<K>;
    { m -= c } ; // -> std::same_as<K&>;
    { c * c } ; // -> std::same_as<K>;
    { m *= c } ; // -> std::same_as<K&>;
  };
};

template<typename K>
concept div_ring = ring<K> && requires(K m, const K c) {
  { c / c } ; // -> std::same_as<K>;
  { m /= c } ; // -> std::same_as<K&>;
};

template<typename K>
concept gcd_ring = div_ring<K> && requires(K m, const K c) {
  { gcd(c, c) } ; // -> std::same_as<K>;
};

template<typename K>
struct is_field_decl : std::false_type {};

template<typename K>
requires requires { typename K::is_field; }
struct is_field_decl<K> : std::true_type {};

template<typename K>
concept field = div_ring<K> && is_field_decl<K>::value;
