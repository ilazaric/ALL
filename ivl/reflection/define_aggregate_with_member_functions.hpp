#pragma once

#include <meta>

namespace ivl::reflection {
template<typename Lambda, typename Parent, size_t /* Unique */>
struct callable {
  // TODO: when P3377 lands, use std::enclosing_cast
  // ....: should also move the memfns to end, so aggregate init is not stupid
  template<typename Self, typename... Args>
  /* constexpr -- can't be atm */ decltype(auto) operator()(this Self&& self, Args&&... args) {
    auto self_ptr = const_cast<callable*>(&self);
    auto parent_ptr = reinterpret_cast<Parent*>(self_ptr);
    return Lambda{}(std::forward_like<Self>(*parent_ptr), std::forward<Args>(args)...);
  }
};

consteval std::meta::info member_function_spec(
  std::string_view name,       //
  std::meta::info lambda_type, //
  std::meta::info parent_type, //
  std::size_t index
) {
  auto callable_type = substitute((^^callable), {lambda_type, parent_type, std::meta::reflect_constant(index)});
  return data_member_spec(callable_type, {.name = name, .no_unique_address = true});
}

struct member_function_description {
  std::string name;
  std::meta::info lambda_type;

  template<typename Lambda>
    requires(!std::same_as<Lambda, std::meta::info>)
  consteval member_function_description(std::string_view name, const Lambda& lambda)
      : name(name), lambda_type(dealias(^^Lambda)) {}

  consteval member_function_description(std::string_view name, std::meta::info lambda_type)
      : name(name), lambda_type(lambda_type) {}
};

template<
  typename Data = std::initializer_list<std::meta::info>,
  typename Fn = std::initializer_list<member_function_description>>
consteval std::meta::info define_aggregate_with_member_functions(
  std::meta::info class_type, //
  Data&& data,                //
  Fn&& fn
) {
  std::vector<std::meta::info> specs;
  size_t index = 0;
  for (auto [name, lambda_type] : fn) {
    specs.push_back(member_function_spec(name, lambda_type, class_type, index));
    ++index;
  }
  specs.insert_range(specs.end(), std::forward<Data>(data));
  return define_aggregate(class_type, specs);
}
} // namespace ivl::reflection
