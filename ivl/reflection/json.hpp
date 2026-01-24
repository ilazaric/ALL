#pragma once

#include <ivl/reflection/util>
#include <nlohmann/json.hpp>
#include <cassert>
#include <meta>

namespace ivl {

struct from_json_impl_tag {};
struct to_json_impl_tag {};

template <typename T, typename Tag>
auto from_to_json_impl(Tag tag, const auto& arg) {
  static_assert(!is_pointer_type(^^T));

  using RetT = std::conditional_t<std::same_as<Tag, to_json_impl_tag>, nlohmann::json, T>;

  if constexpr (!is_class_type(^^T) || std::same_as<T, nlohmann::json> || std::same_as<T, std::string>) {
    if constexpr (std::same_as<Tag, to_json_impl_tag>) return nlohmann::json(arg);
    else return arg.template get<T>();
  } else if constexpr (reflection::is_instantiation_of(^^T, ^^std::vector)) {
    using ElementT = [:template_arguments_of(^^T)[0]:];
    auto ret = RetT{};
    if constexpr (std::same_as<Tag, to_json_impl_tag>) ret = nlohmann::json::array();
    for (auto&& el : arg) ret.emplace_back(from_to_json_impl<ElementT>(tag, el));
    return ret;
  } else if constexpr (reflection::is_instantiation_of(^^T, ^^std::map)) {
    using KeyT = [:template_arguments_of(^^T)[0]:];
    using ValueT = [:template_arguments_of(^^T)[1]:];
    auto ret = RetT{};
    if constexpr (std::same_as<Tag, to_json_impl_tag>) ret = nlohmann::json::object();
    for (auto&& [key, value] : arg) {
      ret.emplace(from_to_json_impl<KeyT>(tag, key), from_to_json_impl<ValueT>(tag, value));
    }
    return ret;
  } else if constexpr (is_class_type(^^T) && !reflection::is_child_of(^^T, ^^std) &&
                       !reflection::is_child_of(^^T, ^^nlohmann)) {
    auto ret = RetT{};
    if constexpr (std::same_as<Tag, to_json_impl_tag>) ret = nlohmann::json::object();
    [:substitute(
        ^^reflection::foreach, reflection::wrap(nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()))
      ):]()([&]<std::meta::info member> {
      if constexpr (std::same_as<Tag, to_json_impl_tag>) {
        ret.emplace(identifier_of(member), from_to_json_impl<typename[:type_of(member):]>(tag, arg.[:member:]));
      } else {
        ret.[:member:] = from_to_json_impl<typename[:type_of(member):]>(tag, arg[identifier_of(member)]);
      }
    });
    return ret;
  } else {
    static_assert(false, "Case not handled!");
  }
}

template <typename T>
T from_json(const nlohmann::json& j) {
  return from_to_json_impl<T>(from_json_impl_tag{}, j);
}

template <typename T>
nlohmann::json to_json(const T& t) {
  return from_to_json_impl<T>(to_json_impl_tag{}, t);
}

// template <typename T>
// nlohmann::json to_json(const T& arg) {
//   static_assert(!is_pointer_type(^^T));

//   if constexpr (!is_class_type(^^T) || std::same_as<T, nlohmann::json> || std::same_as<T, std::string>) {
//     return nlohmann::json(arg);
//   } else if constexpr (is_instantiation_of(^^T, ^^std::vector)) {
//     auto ret = nlohmann::json::array();
//     for (auto&& el : arg) ret.emplace_back(to_json(el));
//     return ret;
//   } else if constexpr (is_instantiation_of(^^T, ^^std::map)) {
//     auto ret = nlohmann::json::object();
//     for (auto&& [key, value] : arg) {
//       auto kj = to_json(key);
//       ret.emplace(kj.is_string() ? kj.template get<std::string>() : kj.dump(), to_json(value));
//     }
//     return ret;
//   } else if constexpr (is_class_type(^^T) && !is_child_of(^^T, ^^std) && !is_child_of(^^T, ^^nlohmann)) {
//     auto ret = nlohmann::json::object();
//     [:substitute(^^foreach, wrap(nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()))):]()(
//       [&]<std::meta::info member> { ret.emplace(identifier_of(member), to_json(arg.[:member:])); }
//     );
//     return ret;
//   } else {
//     static_assert(false);
//   }
// }
} // namespace ivl
