#pragma once

#ifdef IVL_DBG_MODE
# if __cpp_lib_stacktrace >= 202011L
#  include <stacktrace>
#  define IVL_DBG_ASSERT(expr) do {                     \
    if (!(expr)){                                       \
      std::cerr << "[IVL] ERROR: ASSERTION FAILED\n"    \
                <<"[IVL] LINE: " << __LINE__ << "\n"    \
                << "[IVL] EXPR: " << #expr << "\n"      \
                << std::stacktrace::current() << "\n";  \
      exit(-1);                                         \
    }} while (false)
# else
#  define IVL_DBG_ASSERT(expr) do {                     \
    if (!(expr)){                                       \
      std::cerr << "[IVL] ERROR: ASSERTION FAILED\n"    \
                <<"[IVL] LINE: " << __LINE__ << "\n"    \
                << "[IVL] EXPR: " << #expr << "\n";     \
      exit(-1);                                         \
    }} while (false)
# endif
#else
# define IVL_DBG_ASSERT(expr)
#endif
