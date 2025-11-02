#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#define FWD(x) std::forward<decltype(x)>(x)

namespace ivl::util {

  template <typename T>
  constexpr std::string_view typestr() {
    // only works with gcc, only care about gcc atm
    // TODO: add #error on non-gcc
    std::string_view sv = __PRETTY_FUNCTION__;
    sv                  = sv.substr(58);
    sv                  = sv.substr(0, sv.size() - 50);
    return sv;
  }

  constexpr std::string str(auto&&... args) {
    std::stringstream ss;
    // (ss << FWD(args), ...);
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

} // namespace ivl::util
