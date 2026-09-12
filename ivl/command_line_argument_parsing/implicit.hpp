#pragma once

#include "parser_declaration"
#include <algorithm>
#include <format>
#include <meta>
#include <optional>
#include <print>
#include <ranges>
#include <string_view>

namespace ivl::cmdline_parsing {
namespace implicit_detail {
  template<typename>
  struct injection_declaration {
    friend consteval auto injection_function(injection_declaration);
  };

  template<typename T, std::meta::info V>
    requires(V != std::meta::info{}) // null reflection represents "unset"
  struct injection_definition {
    friend consteval auto injection_function(injection_declaration<T>) { return V; }
  };

  template<typename T, typename = decltype([] {})>
  consteval std::meta::info injection_fetch() {
    if constexpr (requires { injection_function(injection_declaration<T>{}); }) {
      return injection_function(injection_declaration<T>{});
    } else {
      return {};
    }
  }

  template<size_t>
  struct index {};

  struct node {
    const char* name_begin;
    size_t name_length;
    std::meta::info type;

    consteval std::string_view name() const { return std::string_view(name_begin, name_length); }
  };

  consteval std::meta::info fetch_raw(size_t i) {
    auto key = substitute((^^index), {std::meta::reflect_constant(i)});
    auto fetch = substitute((^^injection_fetch), {key});
    auto value = extract<std::meta::info (*)()>(fetch)();
    return value;
  }

  consteval size_t size() {
    for (size_t i = 0; true; ++i) {
      auto stored = fetch_raw(i);
      if (stored == std::meta::info{}) return i;
    }
  }

  // precondition: i < implicit_size()
  // not added bc it would do a lot of work
  consteval node fetch(size_t i) { return extract<node>(fetch_raw(i)); }

  // if we could do this with `define_aggregate()` we would not need friend injection
  consteval void store(size_t i, node node) {
    auto key = substitute((^^index), {std::meta::reflect_constant(i)});
    auto value = std::meta::reflect_constant(std::meta::reflect_constant(node));
    auto definer = substitute((^^injection_definition), {key, value});
    size_of(definer); // instantiate it
  }

  consteval size_t register_name(std::string_view name, std::meta::info type, std::source_location loc) {
    auto throw_error = [=]<typename... Ts>(std::format_string<Ts...> fmt, Ts&&... args) {
      auto base = std::format("[implicit] register_name({:?}, {:?})", name, display_string_of(type));
      auto message = std::format(fmt, static_cast<Ts&&>(args)...);
      auto full = std::format("{}: {}", base, message);
      throw std::meta::exception(full, (^^register_name), loc);
    };
    type = dealias(type);
    if (is_reference_type(type)) throw_error("type must not be a reference");
    if (is_const(type)) throw_error("type must not be const qualified");
    if (is_volatile(type)) throw_error("type must not be volatile qualified");
    size_t sz = size();
    for (size_t i = 0; i < sz; ++i) {
      auto node = fetch(i);
      if (node.name() != name) continue;
      if (is_same_type(node.type, type)) return i; // already set to correct value
      throw_error("name already associated with a different type: {:?}", display_string_of(node.type));
    }
    // name doesn't already exist in "container", adding it
    store(
      sz, //
      node{
        .name_begin = name.data(),
        .name_length = name.size(),
        .type = type,
      }
    );
    return sz;
  }

  template<typename T, size_t /* index */>
  struct parsed_storage {
    static inline std::optional<T> value = std::nullopt;
  };

  template<typename T, typename U>
  inline static consteval std::optional<T>* find_value_impl() {
    return &U::value;
  }

  template<typename T>
  struct fixed_name {
    std::optional<T>* value_ptr;

    inline static consteval std::optional<T>* find_value(std::meta::info storage) {
      auto ret = extract<std::optional<T>*(*)()>(substitute(^^find_value_impl, {^^T, storage}))();
      return ret;
    }

    consteval fixed_name(std::string_view name, std::source_location loc = std::source_location::current()) {
      // if `name` is associated with a string literal, we cannot use it
      // as template argument, so laundering it first
      name = std::string_view(std::define_static_string(name));
      size_t index = register_name(name, ^^T, loc);
      auto storage = substitute((^^parsed_storage), {^^T, std::meta::reflect_constant(index)});
      value_ptr = find_value(storage);
    }

    consteval fixed_name(const char* name) : fixed_name(std::string_view(name)) {}
  };

  inline bool parsed = false;
} // namespace implicit_detail

// this is parseable, and should be added to ivl_main() parameter list
struct implicit {};

// so i can do `using namespace ivl::cmdline_parsing::implicit_functions;`
inline namespace implicit_functions {
  template<typename T>
  const std::optional<T>& implicit_optional(implicit_detail::fixed_name<T> id) {
    contract_assert(implicit_detail::parsed);
    return *id.value_ptr;
  }

  template<typename T>
  T implicit_value(implicit_detail::fixed_name<std::remove_cvref_t<T>> name, T&& default_value) {
    contract_assert(implicit_detail::parsed);
    return *name.value_ptr ? **name.value_ptr : static_cast<T&&>(default_value);
  }

  bool implicit_flag(implicit_detail::fixed_name<bool> name, bool default_value = false) {
    return implicit_value(name, default_value);
  }
} // namespace implicit_functions

template<>
struct parser<implicit> {
  template<typename = void>
  inline bool parse(implicit&, raw_arguments& rest) const {
    contract_assert(!implicit_detail::parsed);
    implicit_detail::parsed = true;
    while (!rest.empty()) {
      auto curr = rest[0];
      if (!curr.starts_with("--")) break;
      if (curr == "--") break;
      auto name = curr.substr(2);
      const char* eq = nullptr;
      if (auto loc = name.find('='); loc != std::string_view::npos) {
        eq = name.substr(loc + 1).data();
        name = name.substr(0, loc);
      }
      bool found = false;
      template for (constexpr size_t i : std::views::iota(0ull, implicit_detail::size())) {
        constexpr auto node = implicit_detail::fetch(i);
        if (node.name() != name) continue;
        found = true;
        rest.remove_prefix(1);
        raw_arguments resteq(&eq, &eq + !!eq);
        parser<typename[:node.type:]> p;
        typename[:node.type:] v;
        if (!p.parse(v, eq ? resteq : rest)) {
          std::println("during parsing option: {:?}", curr);
          return false;
        }
        if (eq && !resteq.empty()) {
          std::println("cannot parse payload after `=`: {:?}", curr);
          return false;
        }
        implicit_detail::parsed_storage<typename[:node.type:], i>::value = v;
        break;
      }
      if (!found) break;
    }
    return true;
  }
};
} // namespace ivl::cmdline_parsing
