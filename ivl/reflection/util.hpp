#pragma once

#include <cassert>
#include <meta>

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

// constexpr template for is broken at the moment, this can be used to avoid it.
template <std::meta::info... Is>
auto foreach () {
  return [](auto&& callable) { ((callable.template operator()<Is>()), ...); };
}
} // namespace ivl::reflection
