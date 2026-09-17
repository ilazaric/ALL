#pragma once

// libstdc++ implements formatting of stacktrace_entry and basic_stacktrace
// ... but fmtlib doesn't
// here we plug the hole, mimicking libstdc++ impl

#include "default"
#include <stacktrace>

#ifdef IVL_FMT_VIA_FMT
#include <sstream>
#include <string_view>

template<>
struct ivl::fmt::formatter<std::stacktrace_entry, char> {
  ivl::fmt::formatter<std::string_view> underlying;
  constexpr auto parse(auto& ctx) { return underlying.parse(ctx); }
  auto format(const std::stacktrace_entry& e, auto& ctx) const {
    std::ostringstream os;
    os << e;
    return underlying.format(os.view(), ctx);
  }
};

template<typename Alloc>
struct ivl::fmt::formatter<std::basic_stacktrace<Alloc>, char> {
  using nonlocking = void;
  constexpr auto parse(auto& ctx) {
    if (ctx.begin() == ctx.end() || *ctx.begin() == '}') return ctx.begin();
    throw ivl::fmt::format_error("format_error: invalid format-spec for std::basic_stacktrace");
  }
  auto format(const std::basic_stacktrace<Alloc>& e, auto& ctx) const {
    std::ostringstream os;
    os << e;
    return ivl::fmt::formatter<std::string_view>{}.format(os.view(), ctx);
  }
};

// without this formatter<stacktrace> is ambiguous with fmtlib generic range formatter
// couldn't figure out anything else to kill their impl
template<typename Alloc, typename Char, typename Enable>
struct ivl::fmt::range_format_kind<std::basic_stacktrace<Alloc>, Char, Enable>
    : std::integral_constant<ivl::fmt::range_format, ivl::fmt::range_format::disabled> {};
#endif // IVL_FMT_VIA_FMT
