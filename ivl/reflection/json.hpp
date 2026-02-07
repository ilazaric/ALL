#pragma once

#include <ivl/reflection/util>
#include <nlohmann/json.hpp>
#include <cassert>
#include <map>
#include <meta>
#include <optional>
#include <set>
#include <vector>

namespace ivl {
struct json_serialize_as_array_t {};

constexpr json_serialize_as_array_t json_serialize_as_array;

enum class from_to_json_impl_direction { FROM, TO };

template <
  typename T, from_to_json_impl_direction Direction, bool SerializeAsArray = false,
  typename InputT = std::conditional_t<Direction == from_to_json_impl_direction::TO, T, nlohmann::json>,
  typename RetT = std::conditional_t<Direction == from_to_json_impl_direction::TO, nlohmann::json, T>>
RetT from_to_json_impl(const InputT& arg) {
  static_assert(!is_pointer_type(^^T));
  static_assert(!is_reference_type(^^T));

  using enum from_to_json_impl_direction;

  if constexpr (reflection::is_instantiation_of(^^T, ^^std::optional)) {
    if constexpr (Direction == TO) {
      return arg ? from_to_json_impl<T::value_type, Direction>(*arg) : RetT();
    } else {
      return !arg.is_null() ? from_to_json_impl<T::value_type, Direction>(arg) : RetT();
    }
  } else if constexpr (reflection::is_instantiation_of(^^T, ^^std::chrono::duration)) {
    using RepT = T::rep;
    if constexpr (Direction == TO) return from_to_json_impl<RepT, Direction>(arg.count());
    else return RetT(from_to_json_impl<RepT, Direction>(arg));
  } else if constexpr (reflection::is_instantiation_of(^^T, ^^std::chrono::time_point)) {
    using DurT = T::duration;
    if constexpr (Direction == TO) return from_to_json_impl<DurT, Direction>(arg.time_since_epoch());
    else return RetT(from_to_json_impl<DurT, Direction>(arg));
  } else if constexpr (std::same_as<T, std::filesystem::path>) {
    return from_to_json_impl<std::string, Direction>(arg);
  } else if constexpr (!is_class_type(^^T) || std::same_as<T, nlohmann::json> || std::same_as<T, std::string>) {
    if constexpr (Direction == TO) return nlohmann::json(arg);
    else return arg.template get<T>();
  } else if constexpr (reflection::is_instantiation_of(^^T, ^^std::vector) ||
                       reflection::is_instantiation_of(^^T, ^^std::set)) {
    using ElementT = T::value_type;
    auto ret = RetT{};
    if constexpr (Direction == TO) ret = nlohmann::json::array();
    for (auto&& el : arg) {
      if constexpr (reflection::is_instantiation_of(^^T, ^^std::set) && Direction == FROM)
        ret.emplace(from_to_json_impl<ElementT, Direction>(el));
      else ret.emplace_back(from_to_json_impl<ElementT, Direction>(el));
    }
    return ret;
  } else if constexpr (reflection::is_instantiation_of(^^T, ^^std::map)) {
    using KeyT = [:template_arguments_of(^^T)[0]:];
    using ValueT = [:template_arguments_of(^^T)[1]:];
    auto ret = RetT{};
    if constexpr (SerializeAsArray) {
      if constexpr (Direction == TO) {
        ret = nlohmann::json::array();
        for (auto&& [key, value] : arg)
          ret.emplace_back(
            nlohmann::json::object({
              {"key", from_to_json_impl<KeyT, Direction>(key)},
              {"value", from_to_json_impl<ValueT, Direction>(value)},
            })
          );
      } else {
        for (auto&& kv : arg)
          ret.emplace(from_to_json_impl<KeyT, Direction>(kv["key"]), from_to_json_impl<ValueT, Direction>(kv["value"]));
      }
    } else {
      if constexpr (Direction == TO) {
        ret = nlohmann::json::object();
        for (auto&& [key, value] : arg)
          ret.emplace(from_to_json_impl<KeyT, Direction>(key), from_to_json_impl<ValueT, Direction>(value));
      } else {
        for (auto&& [key, value] : arg.items())
          ret.emplace(
            from_to_json_impl<KeyT, Direction>(nlohmann::json(key)), from_to_json_impl<ValueT, Direction>(value)
          );
      }
    }
    return ret;
  } else if constexpr (is_class_type(^^T) && !reflection::is_child_of(^^T, ^^std) &&
                       !reflection::is_child_of(^^T, ^^nlohmann)) {
    // static_assert(false, display_string_of(^^T));
    auto ret = RetT{};
    if constexpr (Direction == TO) ret = nlohmann::json::object();
    [:substitute(
        ^^reflection::foreach, reflection::wrap(nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()))
      ):]()([&]<std::meta::info member> {
      if constexpr (Direction == TO) {
        ret.emplace(
          identifier_of(member),
          from_to_json_impl<typename[:type_of(member):], Direction,
                                                       !annotations_of_with_type(member, ^^json_serialize_as_array_t)
                                                          .empty()>(arg.[:member:])
        );
      } else {
        ret.[:member:] = from_to_json_impl<
                         typename[:type_of(member):], Direction,
                                                    !annotations_of_with_type(member, ^^json_serialize_as_array_t)
                                                       .empty()>(arg[identifier_of(member)]);
      }
    });
    return ret;
  } else {
    static_assert(false, display_string_of(^^T));
  }
}

template <typename T>
T from_json(const nlohmann::json& j) {
  return from_to_json_impl<T, from_to_json_impl_direction::FROM>(j);
}

template <typename T>
nlohmann::json to_json(const T& t) {
  return from_to_json_impl<T, from_to_json_impl_direction::TO>(t);
}
} // namespace ivl
