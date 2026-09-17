#pragma once

#ifdef IVL_FMT_USE_STD
#define IVL_FMT_VIA_STD
#include <format>
#include <print>

namespace ivl {
namespace fmt = ::std;
} // namespace ivl
#else // !IVL_FMT_USE_STD
#define IVL_FMT_VIA_FMT
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>

namespace ivl {
namespace fmt = ::fmt;
} // namespace ivl
#endif // IVL_FMT_USE_STD
