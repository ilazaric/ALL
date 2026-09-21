#pragma once

#include <ivl/format>
#include <ivl/reflection/prettier_types>
#include <cassert>
#include <meta>

// TODO: maybe move to ivl/reflection/format , add prettier_types to it
// doesnt work with ivl::fmt::print
template<>
struct ivl::fmt_raw::formatter<std::meta::info, char> {
  bool debug = false;
  ivl::fmt_raw::formatter<std::string_view, char> underlying;

  consteval auto parse(auto& ctx) {
    if (ctx.begin() == ctx.end() || *ctx.begin() == '}') return ctx.begin();
    if (*ctx.begin() == '?') {
      debug = true;
      ctx.advance_to(ctx.begin() + 1);
      if (ctx.begin() == ctx.end() || *ctx.begin() == '}') return ctx.begin();
    }
    if (*ctx.begin() == ':') {
      ctx.advance_to(ctx.begin() + 1);
      ctx.advance_to(underlying.parse(ctx));
    }
    assert(ctx.begin() == ctx.end() || *ctx.begin() == '}');
    return ctx.begin();
  }

  consteval void set_debug_format() noexcept {
    debug = true;
    underlying.set_debug_format();
  }

  consteval auto format(std::meta::info i, auto& ctx) const {
    return underlying.format(debug ? ivl::reflection::display_string_of(i) : identifier_of(i), ctx);
  }
};

namespace ivl::reflection {
consteval bool is_instantiation_of(std::meta::info typei, std::meta::info templatei) {
  return has_template_arguments(typei) && template_of(typei) == templatei;
}

consteval bool is_child_of(std::meta::info child, std::meta::info parent) {
  assert(child != parent);
  while (has_parent(child) && child != parent) child = parent_of(child);
  return child == parent;
}

consteval std::vector<std::meta::info> wrap(std::vector<std::meta::info> v) {
  std::vector<std::meta::info> ret;
  for (auto i : v) ret.push_back(reflect_constant(i));
  return ret;
}

consteval auto bases(std::meta::info i) {
  return define_static_array(bases_of(i, std::meta::access_context::unchecked()));
}

consteval auto nsdms(std::meta::info i) {
  return define_static_array(nonstatic_data_members_of(i, std::meta::access_context::unchecked()));
}

consteval auto members(std::meta::info i) {
  return define_static_array(members_of(i, std::meta::access_context::unchecked()));
}

// constexpr template for is broken at the moment, this can be used to avoid it.
template<std::meta::info... Is>
auto foreach () {
  return [](auto&& callable) { ((callable.template operator()<Is>()), ...); };
}

consteval std::meta::info get_nsdm(std::meta::info i, std::string_view name) {
  i = remove_cvref(dealias(i));
  for (auto member : nonstatic_data_members_of(i, std::meta::access_context::unchecked()))
    if (has_identifier(member) && identifier_of(member) == name) return member;
  __builtin_constexpr_diag(2, "", ivl::fmt::format("no member with name {:?} found in {:?}", name, i));
  __builtin_unreachable();
}
} // namespace ivl::reflection
