#pragma once

#include <meta>
#include <string>
#include <type_traits>
#include <utility>

namespace ivl {
template<typename E>
  requires std::is_enum_v<E>
consteval std::span<const E> enumerators() {
  std::vector<E> ret;
  for (auto e : enumerators_of(^^E)) ret.push_back(extract<E>(e));
  return define_static_array(ret);
}

template<typename E>
  requires std::is_enum_v<E>
constexpr bool is_enumerator(E e) {
  for (auto enumerator : enumerators<E>())
    if (e == enumerator) return true;
  return false;
}

template<typename E>
  requires std::is_enum_v<E>
consteval bool all_enumerators_different() {
  auto all = enumerators<E>();
  for (size_t i = 0; i < all.size(); ++i)
    for (size_t j = 0; j < i; ++j)
      if (all[i] == all[j]) return false;
  return true;
}

// TODO: figure out what to do if not all different
template<typename E>
  requires(std::is_enum_v<E> && all_enumerators_different<E>())
constexpr std::string enum_to_string(E e) {
  template for (auto enumerator : define_static_array(enumerators_of(^^E))) {
    if (e == extract<E>(enumerator)) return std::string(identifier_of(enumerator));
  }
  return "<not-enumerator[" + to_string(std::to_underlying(e)) + "]>";
}

// checks if value is equal to one of the enumerators
template<typename E>
  requires std::is_enum_v<E>
struct checked_enum_impl {
  struct type {
    E e;
    constexpr E get() const { return e; }

    type() = delete;

    type(const type&) = default;
    type(type&&) = default;

    type& operator=(const type&) = default;
    type& operator=(type&&) = default;

    explicit constexpr type(E e) pre(is_enumerator(e)) : e(e) {}

    constexpr auto operator<=>(const type&) const = default;
  };

  consteval {
    for (auto er : enumerators_of(^^E)) {
      auto name = identifier_of(er);
      auto value = extract<E>(er);
      ivl_inject_csdm(^^type, name, std::meta::reflect_constant(type(value)));
    }
  }
};

template<typename E>
  requires std::is_enum_v<E>
using checked_enum = checked_enum_impl<E>::type;

template<typename E>
  requires std::is_enum_v<E>
consteval bool all_flag_enumerators_disjoint() {
  auto all = enumerators<E>();
  for (size_t i = 0; i < all.size(); ++i)
    for (size_t j = 0; j < i; ++j)
      if (std::to_underlying(all[i]) & std::to_underlying(all[j])) return false;
  return true;
}

template<typename E>
  requires std::is_enum_v<E>
constexpr bool is_disjunction_of_enumerators(E e) {
  using IE = std::underlying_type_t<E>;
  IE arg_value = std::to_underlying(e);
  IE built_up = 0;
  for (auto enumerator : enumerators<E>()) {
    IE enumerator_value = std::to_underlying(enumerator);
    if ((arg_value & enumerator_value) == enumerator_value) built_up |= enumerator_value;
  }
  return arg_value == built_up;
}

template<typename E>
  requires std::is_enum_v<E>
struct checked_flag_enum_impl {
  struct type {
    E e;
    constexpr E get() const { return e; }

    constexpr type() : e{} {}

    type(const type&) = default;
    type(type&&) = default;

    type& operator=(const type&) = default;
    type& operator=(type&&) = default;

    explicit constexpr type(E e) pre(is_disjunction_of_enumerators(e)) : e(e) {}

    constexpr auto operator<=>(const type&) const = default;

    constexpr type& operator|=(type o) {
      e = static_cast<E>(std::to_underlying(e) | std::to_underlying(o.e));
      return *this;
    }
    constexpr type operator|(type o) const {
      o |= *this;
      return o;
    }

    constexpr type& operator&=(type o)
      requires(all_flag_enumerators_disjoint<E>())
    {
      e = static_cast<E>(std::to_underlying(e) & std::to_underlying(o.e));
      return *this;
    }

    constexpr type operator&(type o) const
      requires(all_flag_enumerators_disjoint<E>())
    {
      o &= *this;
      return o;
    }
  };

  consteval {
    for (auto er : enumerators_of(^^E)) {
      auto name = identifier_of(er);
      auto value = extract<E>(er);
      ivl_inject_csdm(^^type, name, std::meta::reflect_constant(type(value)));
    }
  }
};

template<typename E>
  requires std::is_enum_v<E>
using checked_flag_enum = checked_flag_enum_impl<E>::type;
} // namespace ivl
