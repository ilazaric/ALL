#pragma once

#include <ivl/exception>
#include <ivl/linux/file_descriptor>
#include <ivl/linux/raw_syscalls>
#include <string>
#include <utility>

namespace ivl::linux::throwing_syscalls {

// TODO: this doesn't do a nice job with write()
// ....: it's even leaky

namespace {
  template <typename T>
  auto convert(T arg) {
    if constexpr (std::is_enum_v<T>) {
      return std::to_underlying<T>(arg);
    } else if constexpr (!std::is_pointer_v<T>) {
      return arg;
    } else if constexpr (std::is_same_v<T, char*> || std::is_same_v<T, const char*>) {
      return arg;
    } else {
      return static_cast<const void*>(arg);
    }
  }
} // namespace

// TODO: join up this with raw_syscalls X macro shenanigans
#define X_PARAMS0()
#define X_PARAMS1(t1, a1) t1 a1
#define X_PARAMS2(t1, a1, ...) t1 a1, X_PARAMS1(__VA_ARGS__)
#define X_PARAMS3(t1, a1, ...) t1 a1, X_PARAMS2(__VA_ARGS__)
#define X_PARAMS4(t1, a1, ...) t1 a1, X_PARAMS3(__VA_ARGS__)
#define X_PARAMS5(t1, a1, ...) t1 a1, X_PARAMS4(__VA_ARGS__)
#define X_PARAMS6(t1, a1, ...) t1 a1, X_PARAMS5(__VA_ARGS__)

#define X_CARGS0(P)
#define X_CARGS1(P, t1, a1) P(a1)
#define X_CARGS2(P, t1, a1, ...) P(a1), X_CARGS1(P, __VA_ARGS__)
#define X_CARGS3(P, t1, a1, ...) P(a1), X_CARGS2(P, __VA_ARGS__)
#define X_CARGS4(P, t1, a1, ...) P(a1), X_CARGS3(P, __VA_ARGS__)
#define X_CARGS5(P, t1, a1, ...) P(a1), X_CARGS4(P, __VA_ARGS__)
#define X_CARGS6(P, t1, a1, ...) P(a1), X_CARGS5(P, __VA_ARGS__)

#define X_ID(x) x

#define X_FARGS0(...) 0
#define X_FARGS1(...) EXCEPTION_CONTEXT("args: {}", X_CARGS1(convert, __VA_ARGS__))
#define X_FARGS2(...) EXCEPTION_CONTEXT("args: {} {}", X_CARGS2(convert, __VA_ARGS__))
#define X_FARGS3(...) EXCEPTION_CONTEXT("args: {} {} {}", X_CARGS3(convert, __VA_ARGS__))
#define X_FARGS4(...) EXCEPTION_CONTEXT("args: {} {} {} {}", X_CARGS4(convert, __VA_ARGS__))
#define X_FARGS5(...) EXCEPTION_CONTEXT("args: {} {} {} {} {}", X_CARGS5(convert, __VA_ARGS__))
#define X_FARGS6(...) EXCEPTION_CONTEXT("args: {} {} {} {} {} {}", X_CARGS6(convert, __VA_ARGS__))

#define X(N, name, ...)                                                                                                \
  inline long name(X_PARAMS##N(__VA_ARGS__)) {                                                                         \
    X_FARGS##N(__VA_ARGS__);                                                                                           \
    auto ret = ::ivl::linux::raw_syscalls::name(X_CARGS##N(X_ID __VA_OPT__(, ) __VA_ARGS__));                          \
    if (ret < 0) {                                                                                                     \
      throw ivl::base_exception{std::format("Syscall `" #name "` failed with error code {}", ret)};                    \
    }                                                                                                                  \
    return ret;                                                                                                        \
  }

#include <ivl/linux/syscall_arguments_X>

#undef X_ID

#undef X_PARAMS0
#undef X_PARAMS1
#undef X_PARAMS2
#undef X_PARAMS3
#undef X_PARAMS4
#undef X_PARAMS5
#undef X_PARAMS6

#undef X_CARGS0
#undef X_CARGS1
#undef X_CARGS2
#undef X_CARGS3
#undef X_CARGS4
#undef X_CARGS5
#undef X_CARGS6

#undef X_FARGS0
#undef X_FARGS1
#undef X_FARGS2
#undef X_FARGS3
#undef X_FARGS4
#undef X_FARGS5
#undef X_FARGS6

} // namespace ivl::linux::throwing_syscalls
