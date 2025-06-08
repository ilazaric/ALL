#pragma once

#include <utility>
#include <string_view>

#define FWD(x) std::forward<decltype(x)>(x)

namespace ivl::util {
  
template<typename T>
constexpr std::string_view typestr(){
  // only works with gcc, only care about gcc atm
  std::string_view sv = __PRETTY_FUNCTION__;
  sv = sv.substr(58);
  sv = sv.substr(0, sv.size() - 50);
  return sv;
}

  // namespace detail {
  //   template<typename T>
  //   struct Piper {
  //     T callable;
  //     friend auto operator|(auto&& arg, const Piper& p){return p.callable(arg);}
  //     friend auto operator|(auto&& arg, Piper& p){return p.callable(arg);}
  //     friend auto operator|(auto&& arg, Piper&& p){return std::move(p).callable(arg);}
  //   };
  // } // namespace detail

} // namespace ivl::util

// #define EXTRACT(...) Piper{[](auto&& arg){}}
