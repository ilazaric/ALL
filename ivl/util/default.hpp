#pragma once

#include <cassert>
#include <filesystem>
#include <format>
#include <source_location>
#include <sstream>
#include <stacktrace>
#include <string>
#include <string_view>
#include <utility>

#pragma IVL add_compiler_flags "-g"
#pragma IVL add_compiler_flags_tail "-lstdc++exp"

#define FWD(x) std::forward<decltype(x)>(x)

namespace ivl::util {
template <typename T>
using const_span = std::span<const T>;

template <typename T>
constexpr std::string_view typestr() {
  // only works with gcc, only care about gcc atm
  // TODO: add #error on non-gcc
  // UPDT: std::meta::identifier_of might be good instead of this
  std::string_view sv = __PRETTY_FUNCTION__;
  sv = sv.substr(58);
  sv = sv.substr(0, sv.size() - 50);
  return sv;
}

constexpr std::string str(auto&&... args) {
  std::stringstream ss;
  (ss << ... << FWD(args));
  return std::move(ss).str();
}

template <typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};
template <typename... Ts>
Overload(const Ts&...) -> Overload<Ts...>;

template <typename Fn>
struct [[nodiscard]] scope_exit {
  Fn fn;
  ~scope_exit() { fn(); }
};

template <typename T>
struct convert_t {
  T operator()(auto&& arg) const { return T{FWD(arg)}; }
};

template <typename T>
inline constexpr convert_t<T> convert;

namespace detail {
  template <typename, typename>
  struct lazy_construct_conversion;
  template <typename T, size_t... Is>
  struct lazy_construct_conversion<T, std::index_sequence<Is...>> {
    operator T(this auto&& self) { return T(std::get<Is>(FWD(self).args)...); }
  };
} // namespace detail

// TODO: lazy_construct might not be good terminology, think
template <typename T, typename... Args>
struct lazy_construct_t : detail::lazy_construct_conversion<T, std::make_index_sequence<sizeof...(Args)>> {
  std::tuple<Args...> args;
  lazy_construct_t(Args... args) : args(args...) {}
};

template <typename T>
auto lazy_construct(auto&&... args) {
  return lazy_construct_t<T, decltype(args)...>(FWD(args)...);
}

std::string hex(std::string_view sv) {
  std::string ret(sv.size()*2, '\0');
  auto hexc = [](int x) -> char { return x < 10 ? '0' + x : 'a' + x - 10; };
  for (size_t i = 0; i < sv.size(); ++i) {
    ret[2 * i] = hexc(((unsigned char)sv[i]) / 16);
    ret[2 * i + 1] = hexc(((unsigned char)sv[i]) % 16);
  }
  return ret;
}

std::filesystem::path repo_root() {
  auto root = std::filesystem::canonical("/proc/self/exe");
  while (!exists(root / ".git")) {
    assert(root.has_parent_path());
    root = root.parent_path();
  }
  return root;
}
} // namespace ivl::util

namespace ivl {
struct panic_exception : std::exception {
  std::string msg;
  explicit panic_exception(std::string&& msg) : msg(std::move(msg)) {}
  const char* what() const noexcept override { return msg.c_str(); }
};

#pragma IVL add_compiler_flags_tail "-lstdc++exp"

template <typename... Args>
struct panic {
  [[noreturn]] explicit panic(
    Args&&... args, std::string_view header = "!!! PANIC !!!",
    std::source_location loc = std::source_location::current()
  ) {
    throw panic_exception(
      std::format(
        "{}\n\n{}\n\nin {}:{}\nstacktrace:\n{}\n", header, std::format(FWD(args)...), loc.file_name(), loc.line(),
        std::stacktrace::current()
      )
    );
  }
  operator bool() const noexcept { return true; };
};

template <>
struct panic<> {
  [[noreturn]] explicit panic(
    std::string_view header = "!!! PANIC !!!", std::source_location loc = std::source_location::current()
  ) {
    throw panic_exception(
      std::format("{}\n\nin {}:{}\nstacktrace:\n{}\n", header, loc.file_name(), loc.line(), std::stacktrace::current())
    );
  }
  operator bool() const noexcept { return true; };
};

template <typename... Args>
panic(auto&&, Args&&...) -> panic<std::format_string<Args...>, Args...>;
template <typename = void>
panic() -> panic<>;

template <typename... Args>
struct todo {
  [[noreturn]] explicit todo(Args&&... args, std::source_location loc = std::source_location::current()) {
    panic<Args...>(FWD(args)..., "!!! TODO PANIC !!!", loc);
  }
  operator bool() const noexcept { return true; };
};

template <>
struct todo<> {
  [[noreturn]] explicit todo(std::source_location loc = std::source_location::current()) {
    panic<std::format_string<>>("TODO not implemented", "!!! TODO PANIC !!!", loc);
  }
  operator bool() const noexcept { return true; };
};

template <typename... Args>
todo(auto&&, Args&&...) -> todo<std::format_string<Args...>, Args...>;
template <typename = void>
todo() -> todo<>;
} // namespace ivl
