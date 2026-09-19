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
}
namespace ivl::json {
using value = ::nlohmann::json;
inline auto array() { return ::nlohmann::json::array(); }
inline auto object() { return ::nlohmann::json::object(); }
template<typename T>
auto parse(T&& arg) {
  return ::nlohmann::json::parse(static_cast<T&&>(arg));
}
} // namespace ivl::json
#endif // IVL_FMT_VIA_STD

#ifdef IVL_JSON_VIA_BOOST
#include <boost/json.hpp>
namespace ivl {
namespace json = ::boost::json;
namespace json_owner = ::boost::json;
} // namespace ivl
#endif // IVL_FMT_VIA_FMT
