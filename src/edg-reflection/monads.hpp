#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <experimental/meta>
#include <optional>
#include <string_view>
#include <type_traits>
#include <vector>

#define FWD(x) std::forward<decltype(x)>(x)

namespace ivl::refl {
  using namespace std::literals::string_view_literals;

  consteval bool namechar(char c) {
    if (c >= 'a' && c <= 'z') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == '_') return true;
    return false;
  }

  consteval bool is_identifier(const std::string_view& sv) {
    for (auto c : sv)
      if (!namechar(c)) return false;
    return true;
  }

  struct void_t {}; // bc `void` is incomplete

  template <typename T>
  using Fixup = std::conditional_t<std::is_same_v<void, T>, void_t, T>;

  constexpr decltype(auto) fix_invoke(auto&& fn, auto&&... args) {
    using Ret = decltype(FWD(fn)(FWD(args)...));
    if constexpr (std::is_same_v<Ret, void>) {
      FWD(fn)(FWD(args)...);
      return void_t{};
    } else {
      return FWD(fn)(FWD(args)...);
    }
  }

  consteval bool instantiation_of(std::meta::info temp, std::meta::info type) {
    return has_template_arguments(type) && template_of(type) == temp;
  }

  consteval std::vector<std::meta::info> shallow_public_member_functions_of(std::meta::info type) {
    if (!type_is_class(type)) return {};
    std::vector<std::meta::info> ret;
    for (std::meta::info mem : members_of(type)) {
      if (!is_public(mem)) continue;
      if (!is_function(mem) && !is_function_template(mem)) continue;
      if (is_static_member(mem)) continue;
      if (!has_identifier(mem)) continue;
      if (is_constructor(mem)) continue;
      if (is_destructor(mem)) continue;
      if (!is_identifier(identifier_of(mem))) continue;
      ret.push_back(mem);
    }
    return ret;
  }

  consteval std::vector<std::meta::info> deep_public_member_functions_of(std::meta::info type) {
    if (!type_is_class(type)) return {};
    std::vector<std::meta::info> ret;
    std::vector<std::meta::info> types{type};
    for (size_t idx = 0; idx < types.size(); ++idx) {
      auto add = shallow_public_member_functions_of(types[idx]);
      ret.insert(ret.end(), add.begin(), add.end());
      for (auto base : bases_of(types[idx])) {
        if (!is_public(base)) continue;
        types.push_back(type_of(base));
      }
    }
    return ret;
  }

  consteval void monad_complete(std::meta::info type, std::meta::info under) {
    if (!type_is_class(under)) return;
    std::vector<std::meta::info>  memfns = deep_public_member_functions_of(under);
    std::vector<std::string_view> names;
    for (auto memfn : memfns)
      names.push_back(identifier_of(memfn));
    std::ranges::sort(names);
    auto garbage = std::ranges::unique(names);
    names.erase(garbage.begin(), garbage.end());
    for (auto& name : names) {
      queue_injection(^{
          constexpr decltype(auto) \id("f"sv, name) (this auto&& self, auto&&... args) {
            return FWD(self).monad_map([&](auto&& under){
        return FWD(under).\id(name)(FWD(args)...);
            });
    }
  });
} // namespace ivl::refl
}

template <typename T>
struct Opt : std::optional<T> {
  using std::optional<T>::optional;

  constexpr auto monad_bind(this auto&& self, auto&& fn) -> decltype(FWD(fn)(FWD(self)))
    requires(instantiation_of(^Opt, ^decltype(FWD(fn)(FWD(self)))))
  {
    if (self) {
      return FWD(fn)(FWD(self));
    } else {
      return std::nullopt;
    }
  }

#define monad_expr fix_invoke(FWD(fn), *FWD(self))
  constexpr auto monad_map(this auto&& self, auto&& fn) -> Opt<decltype(monad_expr)> {
    if (self) {
      return Opt<decltype(monad_expr)>{monad_expr};
    } else {
      return std::nullopt;
    }
  }
#undef monad_expr

  consteval { monad_complete(^Opt<T>, ^T); }
};

template <typename T>
struct Vec : std::vector<T> {
  using std::vector<T>::vector;

  constexpr auto monad_bind(this auto&& self, auto&& fn) -> decltype(FWD(fn)(FWD(self)[0]))
    requires(instantiation_of(^Vec, ^decltype(FWD(fn)(FWD(self)[0]))))
  {
    decltype(FWD(fn)(FWD(self)[0])) ret;
    for (auto&& el : FWD(self))
      ret.insert_range(ret.end(), FWD(fn)(FWD(el)));
    return ret;
  }

  constexpr auto monad_map(this auto&& self, auto&& fn) {
    Vec<decltype(fix_invoke(FWD(fn), FWD(self)[0]))> ret;
    for (auto&& el : FWD(self))
      ret.emplace_back(fix_invoke(FWD(fn), FWD(el)));
    return ret;
  }

  consteval { monad_complete(^Vec<T>, ^T); }
};

} // namespace ivl::refl
