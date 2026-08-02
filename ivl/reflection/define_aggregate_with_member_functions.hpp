#pragma once

#include <enclosing_cast>
#include <meta>

namespace ivl::reflection {
template<typename Lambda, typename Parent, size_t Index>
struct callable {
  // TODO: when P3377 lands, use std::enclosing_cast
  // ....: should also move the memfns to end, so aggregate init is not stupid
  // UPDT: implemented it myself
  template<typename Self, typename... Args>
  constexpr decltype(auto) operator()(this Self&& self, Args&&... args) {
    return Lambda{}(
      std::enclosing_cast<&[:nonstatic_data_members_of(^^Parent, std::meta::access_context::unchecked())[Index]:]>(
        static_cast<Self&&>(self)
      ),
      std::forward<Args>(args)...
    );
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
  std::vector<std::meta::info> specs(std::from_range, static_cast<Data&&>(data));
  for (auto [name, lambda_type] : fn)
    specs.push_back(member_function_spec(name, lambda_type, class_type, specs.size()));
  return define_aggregate(class_type, specs);
}
} // namespace ivl::reflection
