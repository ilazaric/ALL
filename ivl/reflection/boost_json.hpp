#pragma once

#include <ivl/json/boost>
#include <ivl/meta>
#include <ivl/reflection/json_annotations>
#include <ivl/reflection/utility>
#include <ivl/utility/hex>
#include <cassert>
#include <map>
#include <meta>
#include <optional>
#include <set>
#include <vector>

namespace ivl::boosty {
enum class from_to_json_impl_direction { FROM, TO };

template<
  typename T, from_to_json_impl_direction Direction, bool SerializeAsArray = false,
  typename InputT = std::conditional_t<Direction == from_to_json_impl_direction::TO, T, boost::json::value>,
  typename RetT = std::conditional_t<Direction == from_to_json_impl_direction::TO, boost::json::value, T>>
RetT from_to_json_impl(const InputT& arg) {
  static_assert(!is_pointer_type(^^T));
  static_assert(!is_reference_type(^^T));

  using enum from_to_json_impl_direction;

  if constexpr (!annotations_of_with_type(^^T, ^^json_serialize_as_bytes_hex_t).empty()) {
    if constexpr (Direction == TO) {
      return boost::json::value(util::hex(std::string_view((const char*)&arg, (const char*)(&arg + 1))));
    } else {
      T ret;
      auto str = util::unhex(arg.template get<std::string>());
      contract_assert(sizeof(ret) == str.size());
      memcpy(&ret, str.c_str(), sizeof(ret));
      return ret;
    }
  } else if constexpr (reflection::is_instantiation_of(^^T, ^^std::variant)) {
    // TODO: technically we could do non-unique as well
    static_assert(extract<bool>(substitute(^^meta::is_unique, template_arguments_of(^^T))));
    template for (constexpr auto VI : define_static_array(template_arguments_of(^^T))) {
      using V = [:dealias(VI):];
      if constexpr (Direction == TO) {
        if (!std::holds_alternative<V>(arg)) continue;
        auto ret = boost::json::object();
        ret["type"] = reflection::display_string_of(VI);
        ret["value"] = from_to_json_impl<V, Direction>(std::get<V>(arg));
        return ret;
      } else {
        if (arg["type"] != reflection::display_string_of(VI)) continue;
        return T(std::in_place_type_t<V>{}, from_to_json_impl<V, Direction>(arg["value"]));
      }
    }
    contract_assert(false);
    throw std::runtime_error("womp womp");
  } else if constexpr (reflection::is_instantiation_of(^^T, ^^std::optional)) {
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
  } else if constexpr (!is_class_type(^^T) || std::same_as<T, boost::json::value> || std::same_as<T, std::string>) {
    if constexpr (Direction == TO) return boost::json::value(arg);
    else return arg.template get<T>();
  } else if constexpr (
    reflection::is_instantiation_of(^^T, ^^std::vector) || reflection::is_instantiation_of(^^T, ^^std::set)
  ) {
    using ElementT = T::value_type;
    auto ret = RetT{};
    if constexpr (Direction == TO) ret = boost::json::array();
    for (auto&& el : arg) {
      if constexpr (Direction == TO) {
        ret.as_array().emplace_back(from_to_json_impl<ElementT, Direction>(el));
      } else {
        if constexpr (reflection::is_instantiation_of(^^T, ^^std::set))
          ret.emplace(from_to_json_impl<ElementT, Direction>(el));
        else ret.emplace_back(from_to_json_impl<ElementT, Direction>(el));
      }
    }
    return ret;
  } else if constexpr (reflection::is_instantiation_of(^^T, ^^std::map)) {
    using KeyT = [:template_arguments_of(^^T)[0]:];
    using ValueT = [:template_arguments_of(^^T)[1]:];
    auto ret = RetT{};
    if constexpr (SerializeAsArray) {
      if constexpr (Direction == TO) {
        ret = boost::json::array();
        for (auto&& [key, value] : arg)
          ret.as_array().emplace_back(
            boost::json::object({
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
        ret = boost::json::object();
        for (auto&& [key, value] : arg)
          ret.as_object().emplace(
            dump(from_to_json_impl<KeyT, Direction>(key)), from_to_json_impl<ValueT, Direction>(value)
          );
      } else {
        for (auto&& [key, value] : arg.items())
          ret.emplace(
            from_to_json_impl<KeyT, Direction>(boost::json::value(key)), from_to_json_impl<ValueT, Direction>(value)
          );
      }
    }
    return ret;
  } else if constexpr (
    is_class_type(^^T) && !reflection::is_child_of(^^T, ^^std) && !is_same_type(^^T, ^^boost::json::value)
  ) {
    static_assert(bases_of(^^T, std::meta::access_context::unchecked()).empty());
    // static_assert(false, display_string_of(^^T));
    auto ret = RetT{};
    if constexpr (Direction == TO) ret = boost::json::object();
    template for (constexpr auto basic_member : reflection::nsdms(^^T)) {
      // TODO: this sucks
      constexpr auto member =
        has_identifier(^^T) && identifier_of(^^T) == "rusage" && is_union_type(type_of(basic_member))
          ? reflection::nsdms(type_of(basic_member))[0]
          : basic_member;
      using X = [:type_of(member):];
      if constexpr (Direction == TO) {
        auto&& mem = arg.[:member:];
        ret.as_object().emplace(
          identifier_of(member),
          from_to_json_impl<X, Direction, !annotations_of_with_type(member, ^^json_serialize_as_array_t).empty()>(mem)
        );
      } else {
        auto&& mem = ret.[:member:];
        mem = from_to_json_impl<X, Direction, !annotations_of_with_type(member, ^^json_serialize_as_array_t).empty()>(
          arg[identifier_of(member)]
        );
      }
    }
    return ret;
  } else {
    static_assert(false, display_string_of(^^T));
  }
}

template<typename T>
T from_json(const boost::json::value& j) {
  return from_to_json_impl<T, from_to_json_impl_direction::FROM>(j);
}

template<typename T>
boost::json::value to_json(const T& t) {
  return from_to_json_impl<T, from_to_json_impl_direction::TO>(t);
}

template<typename T>
T from_json_string(std::string_view sv) {
  return from_json<T>(boost::json::parse(sv));
}

template<typename T>
std::string to_json_string(const T& t) {
  return to_json<T>(t).dump(2);
}
} // namespace ivl::boosty
