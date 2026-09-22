#pragma once

#if defined(IVL_JSON_USE_NLOHMANN) + defined(IVL_JSON_USE_BOOST) >= 2
#error "at most one IVL_JSON_USE_<kind> can be defined"
#endif

#ifdef IVL_JSON_USE_NLOHMANN
#define IVL_JSON_VIA_NLOHMANN
#endif

#ifdef IVL_JSON_USE_BOOST
#define IVL_JSON_VIA_BOOST
#endif

#if defined(IVL_JSON_USE_NLOHMANN) + defined(IVL_JSON_USE_BOOST) == 0
#define IVL_JSON_VIA_NLOHMANN
#endif

#ifdef IVL_JSON_VIA_NLOHMANN
#include <nlohmann/json.hpp>
namespace ivl {
namespace json_owner = ::nlohmann;
} // namespace ivl
namespace ivl::json {
struct value {
  ::nlohmann::json underlying;

  value(const value&) = default;
  value(value&&) = default;

  value& operator=(const value&) = default;
  value& operator=(value&&) = default;

  operator const ::nlohmann::json&() const { return underlying; }

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
    return underlying.emplace(static_cast<Ts&&>(args)...).second;
  }
};

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
void dump_impl(value const& jv, std::string& out, size_t indent, size_t inc) {
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
  if (indent == static_cast<size_t>(-1)) return ::boost::json::serialize(v);
  contract_assert((indent >> 63) == 0);
  std::string ret;
  dump_impl(v, ret, 0, indent);
  return ret;
}
} // namespace ivl::json
#endif // IVL_JSON_VIA_BOOST
