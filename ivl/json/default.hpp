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

// default == boost
#if defined(IVL_JSON_USE_NLOHMANN) + defined(IVL_JSON_USE_BOOST) == 0
#define IVL_JSON_VIA_BOOST
#endif

#ifdef IVL_JSON_VIA_NLOHMANN
#include <nlohmann/json.hpp>
namespace ivl {
namespace json_owner = ::nlohmann;
} // namespace ivl
namespace ivl::json {
using value = ::nlohmann::json;
inline decltype(auto) array(::nlohmann::json::initializer_list_t init = {}) { return ::nlohmann::json::array(init); }
inline decltype(auto) object(::nlohmann::json::initializer_list_t init = {}) { return ::nlohmann::json::object(init); }
template<typename T>
decltype(auto) parse(T&& arg) {
  return ::nlohmann::json::parse(static_cast<T&&>(arg));
}
decltype(auto) diff(const value& left, const value& right) { return ::nlohmann::json::diff(left, right); }
decltype(auto) dump(const value& v, size_t indent = -1) { return v.dump(indent); }
} // namespace ivl::json
#endif // IVL_FMT_VIA_STD

#ifdef IVL_JSON_VIA_BOOST
#include <boost/json.hpp>
namespace ivl {
namespace json_owner = ::boost::json;
} // namespace ivl
namespace ivl::json {
using value = ::boost::json::value;
using array = ::boost::json::array;
using object = ::boost::json::object;
template<typename T>
decltype(auto) parse(T&& arg) {
  return ::boost::json::parse(static_cast<T&&>(arg));
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

  case ::boost::json::kind::string: {
    out += ::boost::json::serialize(jv.get_string());
    break;
  }

  case ::boost::json::kind::uint64:
  case ::boost::json::kind::int64:
  case ::boost::json::kind::double_:
    out += ::boost::json::serialize(jv);
    break;

  case ::boost::json::kind::bool_:
    if (jv.get_bool()) out += "true";
    else out += "false";
    break;

  case ::boost::json::kind::null:
    out += "null";
    break;
  }
}

std::string dump(const value& v, size_t indent = static_cast<size_t>(-1)) {
  if (indent == static_cast<size_t>(-1)) return ::boost::json::serialize(v);
  std::string ret;
  dump_impl(v, ret, 0, indent);
  return ret;
}
} // namespace ivl::json
#endif // IVL_FMT_VIA_FMT
