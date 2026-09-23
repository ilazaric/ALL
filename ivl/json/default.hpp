#pragma once

#if defined(IVL_JSON_USE_NLOHMANN) + defined(IVL_JSON_USE_BOOST) + defined(IVL_JSON_USE_NLOHMANN_ALIAS) >= 2
#error "at most one IVL_JSON_USE_<kind> can be defined"
#endif

#ifdef IVL_JSON_USE_NLOHMANN
#define IVL_JSON_VIA_NLOHMANN
#endif

#ifdef IVL_JSON_USE_BOOST
#define IVL_JSON_VIA_BOOST
#endif

#ifdef IVL_JSON_USE_NLOHMANN_ALIAS
#define IVL_JSON_VIA_NLOHMANN_ALIAS
#endif

#if defined(IVL_JSON_USE_NLOHMANN) + defined(IVL_JSON_USE_BOOST) + defined(IVL_JSON_USE_NLOHMANN_ALIAS) == 0
#define IVL_JSON_VIA_NLOHMANN_ALIAS
#endif

#ifdef IVL_JSON_VIA_NLOHMANN_ALIAS
#include <nlohmann/json.hpp>
namespace ivl {
namespace json_owner = ::nlohmann;
} // namespace ivl
namespace ivl::json {
using value = ::nlohmann::json;
inline value array(::nlohmann::json::initializer_list_t init = {}) { return ::nlohmann::json::array(init); }
inline value object(::nlohmann::json::initializer_list_t init = {}) { return ::nlohmann::json::object(init); }
template<typename T>
value parse(T&& arg) {
  return ::nlohmann::json::parse(static_cast<T&&>(arg));
}
value diff(const value& left, const value& right) { return ::nlohmann::json::diff(left, right); }
} // namespace ivl::json
#endif // IVL_JSON_VIA_NLOHMANN_ALIAS

#ifdef IVL_JSON_VIA_NLOHMANN
#error "unfinished, and boring, TODO"
#include <nlohmann/json.hpp>
namespace ivl {
namespace json_owner = ::nlohmann;
} // namespace ivl
namespace ivl::json {
struct value;

template<typename T>
decltype(auto) degrade(T&& arg) {
  if constexpr (std::is_same_v<value, std::remove_cvref_t<T>>) {
    if constexpr (std::is_same_v<T, value&>) return reinterpret_cast<::nlohmann::json&>(arg);
    else if constexpr (std::is_same_v<T, const value&>) return reinterpret_cast<const ::nlohmann::json&>(arg);
    else if constexpr (std::is_same_v<T, value>)
      return static_cast<::nlohmann::json&&>(reinterpret_cast<::nlohmann::json&>(arg));
    else if constexpr (std::is_same_v<T, const value>)
      return static_cast<const ::nlohmann::json&&>(reinterpret_cast<const ::nlohmann::json&>(arg));
    else static_assert(false);
  } else {
    return static_cast<T&&>(arg);
  }
}

struct value {
  ::nlohmann::json underlying;

  bool operator==(const value&) const = default;

  value(const value&) = default;
  value(value&&) = default;

  value& operator=(const value&) = default;
  value& operator=(value&&) = default;

  operator const ::nlohmann::json&() const { return underlying; }
  // operator ::nlohmann::json&&() && { return static_cast<::nlohmann::json&&>(underlying); }
  // operator ::nlohmann::json&() & { return underlying; }
  // operator ::nlohmann::json() const { return underlying; }

  template<typename T>
  operator T() const {
    return T{underlying};
  }

  decltype(auto) dump(int indent = -1) const {
    contract_assert(indent >= -1);
    return underlying.dump(indent);
  }

  // UB, dont care
  value& operator[](size_t i) & { return reinterpret_cast<value&>(underlying[i]); }
  value&& operator[](size_t i) && { return std::move(reinterpret_cast<value&>(underlying[i])); }
  const value& operator[](size_t i) const { return reinterpret_cast<const value&>(underlying[i]); }

  value& operator[](std::string_view i) & { return reinterpret_cast<value&>(underlying[i]); }
  value&& operator[](std::string_view i) && { return std::move(reinterpret_cast<value&>(underlying[i])); }
  const value& operator[](std::string_view i) const { return reinterpret_cast<const value&>(underlying[i]); }

  template<typename T>
  value& operator=(T&& arg) {
    underlying = static_cast<T&&>(arg);
    return *this;
  }

  template<typename... Ts>
  bool emplace(Ts&&... args) {
    return underlying.emplace(degrade(static_cast<Ts&&>(args))...).second;
  }

  template<typename... Ts>
  value& emplace_back(Ts&&... args) {
    return reinterpret_cast<value&>(underlying.emplace_back(degrade(static_cast<Ts&&>(args))...));
  }

  template<typename T>
  decltype(auto) get() {
    if constexpr (std::is_same_v<T, value>) {
      return value{underlying.get<::nlohmann::json>()};
    } else {
      return underlying.get<T>();
    }
  }
};

std::ostream& operator<<(std::ostream& out, const value& v) { return out << v.underlying; }

decltype(auto) to_string(const value& v) { return to_string(v.underlying); }

inline value array(::nlohmann::json::initializer_list_t init = {}) { return value{::nlohmann::json::array(init)}; }
inline value object(::nlohmann::json::initializer_list_t init = {}) { return value{::nlohmann::json::object(init)}; }

template<typename T>
value parse(T&& arg) {
  return value{::nlohmann::json::parse(static_cast<T&&>(arg))};
}

value diff(const value& left, const value& right) { return value{::nlohmann::json::diff(left, right)}; }
} // namespace ivl::json
#endif // IVL_JSON_VIA_NLOHMANN

#ifdef IVL_JSON_VIA_BOOST
#include <boost/json.hpp>
namespace ivl {
namespace json_owner = ::boost::json;
} // namespace ivl
namespace ivl::json {
struct value {
  ::boost::json::value underlying;
};
struct value_ref {
  ::boost::json::value_ref underlying;
  value_ref(auto&&... args) : underlying() {}
};
template<typename T>
value parse(T&& arg) {
  return value{::boost::json::parse(static_cast<T&&>(arg))};
}

// https://www.boost.org/doc/libs/latest/libs/json/doc/html/examples.html#pretty
void dump_impl(::boost::json::value const& jv, std::string& out, size_t indent, size_t inc) {
  switch (jv.kind()) {
  case ::boost::json::kind::object: {
    out += "{\n";
    indent += inc;
    auto const& obj = jv.get_object();
    if (!obj.empty()) {
      auto it = obj.begin();
      for (;;) {
        out.append(indent, ' ');
        out += ::boost::json::serialize(it->key());
        out += ": ";
        dump_impl(it->value(), out, indent, inc);
        if (++it == obj.end()) break;
        out += ",\n";
      }
    }
    out += '\n';
    indent -= inc;
    out.append(indent, ' ');
    out += '}';
    break;
  }

  case ::boost::json::kind::array: {
    out += "[\n";
    indent += inc;
    auto const& arr = jv.get_array();
    if (!arr.empty()) {
      auto it = arr.begin();
      for (;;) {
        out.append(indent, ' ');
        dump_impl(*it, out, indent, inc);
        if (++it == arr.end()) break;
        out += ",\n";
      }
    }
    out += '\n';
    indent -= inc;
    out.append(indent, ' ');
    out += ']';
    break;
  }

  default:
    out += ::boost::json::serialize(jv);
    break;
  }
}

std::string dump(const value& v, size_t indent = static_cast<size_t>(-1)) {
  if (indent == static_cast<size_t>(-1)) return ::boost::json::serialize(v.underlying);
  contract_assert((indent >> 63) == 0);
  std::string ret;
  dump_impl(v.underlying, ret, 0, indent);
  return ret;
}
} // namespace ivl::json
#endif // IVL_JSON_VIA_BOOST
