#pragma once

#define IVL_JSON_USE_NLOHMANN // for now, TODO

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
} // namespace ivl::json
#endif // IVL_FMT_VIA_FMT
