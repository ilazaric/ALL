#pragma once

#if defined(IVL_FMT_USE_STD) + defined(IVL_FMT_USE_FMT) >= 2
#error "at most one IVL_FMT_USE_<kind> can be defined"
#endif

#ifdef IVL_FMT_USE_STD
#define IVL_FMT_VIA_STD
#endif

#ifdef IVL_FMT_USE_FMT
#define IVL_FMT_VIA_FMT
#endif

// default == fmtlib
#if defined(IVL_FMT_USE_STD) + defined(IVL_FMT_USE_FMT) == 0
#define IVL_FMT_VIA_FMT
#endif

#ifdef IVL_FMT_VIA_STD
#include <format>
#include <print>
namespace ivl {
namespace fmt = ::std;
} // namespace ivl
#endif // IVL_FMT_VIA_STD

#ifdef IVL_FMT_VIA_FMT
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
namespace ivl {
namespace fmt = ::fmt;
} // namespace ivl
#endif // IVL_FMT_VIA_FMT
