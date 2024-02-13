#pragma once

// #ifndef IVL_DBG_BACKTRACE_BUFFER_SIZE
// # define IVL_DBG_BACKTRACE_BUFFER_SIZE 20
// #endif

// #ifndef LOG
// # define LOG(...)
// #endif

#include <ivl/logger>

#ifdef IVL_DBG_MODE
# if __cpp_lib_stacktrace >= 202011L
#  include <iostream>
#  include <stacktrace>
#  define IVL_DBG_ASSERT(expr, ...) do {                                  \
    if (!(expr)) [[unlikely]] {                                       \
      std::cerr << "[IVL] ERROR: ASSERTION FAILED\n"    \
                <<"[IVL] LINE: " << __LINE__ << "\n"    \
                << "[IVL] EXPR: " << #expr << "\n"      \
                << std::stacktrace::current() << "\n";  \
      LOG("data" __VA_OPT__(,) __VA_ARGS__);            \
      exit(-1);                                         \
    }} while (false)
# else
#  include <iostream>
#  define BOOST_STACKTRACE_USE_ADDR2LINE
#  include <boost/stacktrace.hpp>
#  define IVL_DBG_ASSERT(expr, ...) do {                                  \
    if (!(expr)) [[unlikely]] {                                       \
      std::cerr << "[IVL] ERROR: ASSERTION FAILED\n"    \
                <<"[IVL] LINE: " << __LINE__ << "\n"    \
                << "[IVL] EXPR: " << #expr << "\n";     \
      std::cerr << "[IVL] stacktrace:\n"                                \
                << boost::stacktrace::stacktrace() << std::endl;        \
      LOG("data" __VA_OPT__(,) __VA_ARGS__);                            \
      exit(-1);                                         \
    }} while (false)
# endif
#else
# define IVL_DBG_ASSERT(expr)
#endif
