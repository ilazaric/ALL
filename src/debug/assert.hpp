#pragma once

// #ifndef IVL_DBG_BACKTRACE_BUFFER_SIZE
// # define IVL_DBG_BACKTRACE_BUFFER_SIZE 20
// #endif

// #ifndef LOG
// # define LOG(...)
// #endif

#include <ivl/logger>

#ifdef IVL_DBG_MODE

#if __has_include(<stacktrace>)
#include <stacktrace>
#define IVL_DBG_ASSERT_STACKTRACE                                                                  \
  std::cerr << "[IVL] stacktrace:\n" << std::stacktrace::current() << "\n"
#elif __has_include(<boost/stacktrace.hpp>)
#include <boost/stacktrace.hpp>
#define IVL_DBG_ASSERT_STACKTRACE                                                                  \
  std::cerr << "[IVL] stacktrace:\n" << boost::stacktrace::stacktrace() << "\n"
#else
#define IVL_DBG_ASSERT_STACKTRACE
#endif

#include <iostream>
#define IVL_DBG_ASSERT(expr, ...)                                                                  \
  do {                                                                                             \
    if (!(expr)) [[unlikely]] {                                                                    \
      std::cerr << "[IVL] ERROR: ASSERTION FAILED\n"                                               \
                << "[IVL] LINE: " << __LINE__ << "\n"                                              \
                << "[IVL] EXPR: " << #expr << "\n";                                                \
      IVL_DBG_ASSERT_STACKTRACE;                                                                   \
      LOG("data" __VA_OPT__(, ) __VA_ARGS__);                                                      \
      exit(-1);                                                                                    \
    }                                                                                              \
  } while (false)

#else // IVL_DBG_MODE
#define IVL_DBG_ASSERT(...) (0)
#endif // IVL_DBG_MODE
