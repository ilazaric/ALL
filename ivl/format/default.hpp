#pragma once

#if defined(IVL_FMT_USE_STD) + defined(IVL_FMT_USE_FMT) + defined(IVL_FMT_USE_FMT_MANUAL) >= 2
#error "at most one IVL_FMT_USE_<kind> can be defined"
#endif

#ifdef IVL_FMT_USE_STD
#define IVL_FMT_VIA_STD
#endif

#ifdef IVL_FMT_USE_FMT
#define IVL_FMT_VIA_FMT
#endif

#ifdef IVL_FMT_USE_FMT_MANUAL
#define IVL_FMT_VIA_FMT_MANUAL
#endif

// default == fmtlib
#if defined(IVL_FMT_USE_STD) + defined(IVL_FMT_USE_FMT) + defined(IVL_FMT_USE_FMT_MANUAL) == 0
#define IVL_FMT_VIA_FMT
#endif

#ifdef IVL_FMT_VIA_STD
#include <format>
#include <print>
namespace ivl {
namespace fmt = ::std;
namespace fmt_raw = ::std;
} // namespace ivl
#endif // IVL_FMT_VIA_STD

#ifdef IVL_FMT_VIA_FMT
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
namespace ivl {
namespace fmt = ::fmt;
namespace fmt_raw = ::fmt;
} // namespace ivl
#endif // IVL_FMT_VIA_FMT

#ifdef IVL_FMT_VIA_FMT_MANUAL
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <meta>
namespace ivl {
namespace fmt_raw = ::fmt;
} // namespace ivl
namespace ivl::fmt {
namespace detail {
  template<const char* ptr, size_t len>
  struct wrapper : ::fmt::compiled_string {
    using char_type = char;
    constexpr explicit operator ::fmt::string_view() const { return ::fmt::string_view(ptr, len); }
  };
  template<typename S, typename... Ts>
  consteval std::string format(Ts... args) {
    static_assert(::fmt::is_compiled_string<S>::value);
    return ::fmt::format(S{}, args...);
  }
  template<typename S, typename T, typename... Ts>
  consteval std::string format_to(T arg, Ts... args) {
    static_assert(::fmt::is_compiled_string<S>::value);
    return ::fmt::format_to(arg, S{}, args...);
  }
} // namespace detail
template<typename... Ts>
using format_string = ::fmt::format_string<Ts...>;
using format_error = ::fmt::format_error;
template<typename... Ts>
inline constexpr std::string format(::fmt::format_string<Ts...> fmt, Ts&&... args) {
  if consteval {
    auto p = std::define_static_string(fmt.str);
    auto t =
      substitute((^^detail::wrapper), {std::meta::reflect_constant(p), std::meta::reflect_constant(fmt.str.size())});
    auto f = substitute((^^detail::format), {t, ^^Ts&... });
    return extract<std::string (*)(Ts&...)>(f)(args...);
  } else {
    return ::fmt::format(fmt, static_cast<Ts&&>(args)...);
  }
}
template<typename T, typename... Ts>
inline constexpr std::remove_cvref_t<T> format_to(T&& out, ::fmt::format_string<Ts...> fmt, Ts&&... args) {
  if consteval {
    auto p = std::define_static_string(fmt.str);
    auto t =
      substitute((^^detail::wrapper), {std::meta::reflect_constant(p), std::meta::reflect_constant(fmt.str.size())});
    auto f = substitute((^^detail::format_to), {t, ^^T&, ^^Ts&... });
    return extract<std::remove_cvref_t<T> (*)(T&, Ts&...)>(f)(out, args...);
  } else {
    return ::fmt::format_to(static_cast<T&&>(out), fmt, static_cast<Ts&&>(args)...);
  }
}
template<typename... Ts>
inline void print(::fmt::format_string<Ts...> fmt, Ts&&... args) {
  return ::fmt::print(fmt, static_cast<Ts&&>(args)...);
}
template<typename... Ts>
inline void print(FILE* f, ::fmt::format_string<Ts...> fmt, Ts&&... args) {
  return ::fmt::print(f, fmt, static_cast<Ts&&>(args)...);
}
template<typename... Ts>
inline void println(::fmt::format_string<Ts...> fmt, Ts&&... args) {
  return ::fmt::println(fmt, static_cast<Ts&&>(args)...);
}
template<typename... Ts>
inline void println(FILE* f, ::fmt::format_string<Ts...> fmt, Ts&&... args) {
  return ::fmt::println(f, fmt, static_cast<Ts&&>(args)...);
}
inline void println(FILE* f) { ::fmt::println(f, ""); }
inline void println() { ::fmt::println(""); }
} // namespace ivl::fmt
#endif // IVL_FMT_VIA_FMT_MANUAL
