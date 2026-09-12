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
struct implicit {
  inline static bool parsed = false;
};

template<size_t>
struct implicit_registry;

  template<size_t registry_index>
  struct implicit_find {
    friend consteval auto implicit_injected(implicit_registry<registry_index>&);
    static consteval std::meta::info get() {
      try {
        return return_type_of(^^implicit_injected);
      } catch (...) {
        return {};
      }
    }
  };

template<typename T, const char* /* name_start */, size_t /* name_length */, size_t registry_index>
struct implicit_storage {
  inline static std::optional<T> value = std::nullopt;
  friend consteval implicit_storage implicit_injected(implicit_registry<registry_index>&);
};

template<typename T>
struct implicit_name {
  std::optional<T>* value;

  inline static consteval std::optional<T>* find_value(std::meta::info storage) {
    for (auto member : members_of(storage, std::meta::access_context::unchecked())) {
      if (!has_identifier(member)) continue;
      if (identifier_of(member) != "value") continue;
      return &extract<std::optional<T>&>(member);
    }
    contract_assert(false);
  }

  consteval implicit_name(std::string_view name) {
    for (size_t i = 0; true; ++i) {
      __builtin_constexpr_diag(32, "", "first");
      auto curr = substitute((^^implicit_registry), {std::meta::reflect_constant(i)});
      if (!is_complete_type(curr)) {
        __builtin_constexpr_diag(32, "", "second");
        auto x = std::meta::reflect_constant(name.data());
        __builtin_constexpr_diag(32, "", "second 2");
        auto y = std::meta::reflect_constant(name.size());
        __builtin_constexpr_diag(32, "", "second 3");
        auto storage = substitute(
          (^^implicit_storage), //
          {
            (^^T),
            x,
            y,
            std::meta::reflect_constant(i),
          }
        );
        __builtin_constexpr_diag(32, "", "third");
        define_aggregate(curr, {data_member_spec(storage, {.name = "storage"})});
        value = find_value(storage);
        break;
      }
      auto storage = nonstatic_data_members_of(curr, std::meta::access_context::unchecked())[0];
      auto storage_args = template_arguments_of(storage);
      std::string_view storage_name(extract<const char*>(storage_args[1]), extract<size_t>(storage_args[2]));
      if (name != storage_name) continue;
      if (is_same_type((^^T), storage_args[0])) {
        value = find_value(storage);
        break;
      }
      throw std::meta::exception(
        std::format(
          "cmdline_parsing::implicit: registered name {:?} with different types: {:?} != {:?}", name,
          display_string_of(storage_args[0]), display_string_of(^^T)
        ),
        ^^implicit_name
      );
    }
    __builtin_constexpr_diag(32, "", "end");
  }

  consteval implicit_name(const char* name)
      : implicit_name(std::string_view(std::define_static_string(std::string_view(name)))) {}
};

template<typename T>
T& implicit_get(implicit_name<T> id) {
  contract_assert(implicit::parsed);
  contract_assert(*id.value);
  return **id.value;
}

template<typename T>
std::optional<T&> implicit_get_opt(implicit_name<T> id) {
  contract_assert(implicit::parsed);
  if (!*id.value) return std::nullopt;
  return std::optional<T&>(**id.value);
}

bool implicit_flag(implicit_name<bool> id) {
  contract_assert(implicit::parsed);
  contract_assert(*id.value);
  return **id.value;
}

template<>
struct parser<implicit> {
  inline static consteval size_t icount() {
    for (size_t i = 0; true; ++i) {
      auto curr = substitute((^^implicit_registry), {std::meta::reflect_constant(i)});
      if (!is_complete_type(curr)) return i;
    }
  }

  inline static consteval std::string_view iname(size_t i) {
    auto registry = substitute((^^implicit_registry), {std::meta::reflect_constant(i)});
    auto storage = nonstatic_data_members_of(registry, std::meta::access_context::unchecked())[0];
    auto storage_args = template_arguments_of(registry);
    std::string_view storage_name(extract<const char*>(storage_args[1]), extract<size_t>(storage_args[2]));
    return storage_name;
  }

  inline static consteval std::meta::info itype(size_t i) {
    auto registry = substitute((^^implicit_registry), {std::meta::reflect_constant(i)});
    auto storage = nonstatic_data_members_of(registry, std::meta::access_context::unchecked())[0];
    auto storage_args = template_arguments_of(registry);
    return storage_args[0];
  }

  inline bool parse(implicit&, raw_arguments& rest) const {
    contract_assert(!implicit::parsed);
    implicit::parsed = true;
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
      template for (constexpr size_t i : std::views::iota(0ull, icount())) {
        if (iname(i) != name) continue;
        found = true;
        rest.remove_prefix(1);
        raw_arguments resteq(&eq, &eq + !!eq);
        parser<typename[:itype(i):]> p;
        typename[:itype(i):] v;
        if (!p.parse(v, eq ? resteq : rest)) {
          std::println("during parsing option: {:?}", curr);
          return false;
        }
        if (eq && !resteq.empty()) {
          std::println("cannot parse payload after `=`: {:?}", curr);
          return false;
        }
        break;
      }
      if (!found) break;
    }
    return true;
  }
};
} // namespace ivl::cmdline_parsing
