#pragma once

#include <format>
#include <string>
#include <string_view>

// https://en.wikipedia.org/wiki/ANSI_escape_code#Select_Graphic_Rendition_parameters
// https://en.wikipedia.org/wiki/ANSI_escape_code#24-bit
// `ansi2txt` can strip out colors

namespace ivl::terminal_graphical_rendition {
namespace detail {
  template <typename Wrapped, typename Fmt, typename... Args>
  struct fmt_wrapper {
    Wrapped& wrapped;
    Fmt fmt;
    std::tuple<Args&...> args;
  };

  struct fmt_wrapper_enabled {
    template <typename Self, typename... Args>
      requires(sizeof...(Args) > 0)
    auto operator()(this Self&& self, std::format_string<Args&...> fmt, Args&&... args) {
      return fmt_wrapper<std::remove_reference_t<Self>, std::format_string<Args&...>, Args&...>(
        self, fmt, std::tuple<Args&...>(args...)
      );
    }
  };

  template <typename Wrapped, typename Arg>
  struct single_wrapper {
    Wrapped& wrapped;
    Arg& arg;
  };

  struct single_wrapper_enabled {
    template <typename Self, typename Arg>
    auto operator()(this Self&& self, Arg&& arg) {
      return single_wrapper<std::remove_reference_t<Self>, Arg>(self, arg);
    }
  };

  struct wrapper_enabled : fmt_wrapper_enabled, single_wrapper_enabled {
    using fmt_wrapper_enabled::operator();
    using single_wrapper_enabled::operator();
  };
} // namespace detail

struct color {
  uint8_t r, g, b;
};

struct foreground_color : color, detail::wrapper_enabled {};
struct background_color : color, detail::wrapper_enabled {};
struct foreground_reset {};
struct background_reset {};

namespace colors {
  inline constexpr foreground_color FG_RED{255, 0, 0};
  inline constexpr foreground_color FG_CYAN{0, 255, 255};
  inline constexpr foreground_color FG_LIGHTGREEN{144, 238, 144};
} // namespace colors
} // namespace ivl::terminal_graphical_rendition

template <>
struct std::formatter<ivl::terminal_graphical_rendition::foreground_reset, char> {
  constexpr auto parse(auto& ctx) { return ctx.begin(); }
  auto format(ivl::terminal_graphical_rendition::foreground_reset, auto& ctx) const {
    return std::format_to(ctx.out(), "\x1B[39m");
  }
};

template <>
struct std::formatter<ivl::terminal_graphical_rendition::background_reset, char> {
  constexpr auto parse(auto& ctx) { return ctx.begin(); }
  auto format(ivl::terminal_graphical_rendition::background_reset, auto& ctx) const {
    return std::format_to(ctx.out(), "\x1B[49m");
  }
};

template <>
struct std::formatter<ivl::terminal_graphical_rendition::foreground_color, char> {
  constexpr auto parse(auto& ctx) { return ctx.begin(); }
  auto format(ivl::terminal_graphical_rendition::foreground_color clr, auto& ctx) const {
    return std::format_to(ctx.out(), "\x1B[38;2;{};{};{}m", clr.r, clr.g, clr.b);
  }
  auto format_reset(auto& ctx) const {
    return std::formatter<ivl::terminal_graphical_rendition::foreground_reset, char>{}.format({}, ctx);
  }
};

template <>
struct std::formatter<ivl::terminal_graphical_rendition::background_color, char> {
  constexpr auto parse(auto& ctx) { return ctx.begin(); }
  auto format(ivl::terminal_graphical_rendition::background_color clr, auto& ctx) const {
    return std::format_to(ctx.out(), "\x1B[48;2;{};{};{}m", clr.r, clr.g, clr.b);
  }
  auto format_reset(auto& ctx) const {
    return std::formatter<ivl::terminal_graphical_rendition::background_reset, char>{}.format({}, ctx);
  }
};

template <typename Wrapped, typename Fmt, typename... Args>
struct std::formatter<ivl::terminal_graphical_rendition::detail::fmt_wrapper<Wrapped, Fmt, Args...>, char> {
  constexpr auto parse(auto& ctx) { return ctx.begin(); }
  auto format(ivl::terminal_graphical_rendition::detail::fmt_wrapper<Wrapped, Fmt, Args...> wrap, auto& ctx) const {
    std::formatter<std::remove_const_t<Wrapped>> fmt_wrap;
    ctx.advance_to(fmt_wrap.format(wrap.wrapped, ctx));
    auto&& [... args] = wrap.args;
    ctx.advance_to(std::format_to(ctx.out(), wrap.fmt, args...));
    return fmt_wrap.format_reset(ctx);
  }
};

template <typename Wrapped, typename Arg>
struct std::formatter<ivl::terminal_graphical_rendition::detail::single_wrapper<Wrapped, Arg>, char> {
  std::formatter<std::decay_t<Arg>> underlying;
  constexpr auto parse(auto& ctx) { return underlying.parse(ctx); }
  auto format(ivl::terminal_graphical_rendition::detail::single_wrapper<Wrapped, Arg> wrap, auto& ctx) const {
    std::formatter<std::remove_const_t<Wrapped>> fmt_wrap;
    ctx.advance_to(fmt_wrap.format(wrap.wrapped, ctx));
    ctx.advance_to(underlying.format(wrap.arg, ctx));
    return fmt_wrap.format_reset(ctx);
  }
};
