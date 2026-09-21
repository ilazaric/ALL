#include "stacktrace"

#ifndef IVL_FMT_VIA_FMT
#error "expected formatting via fmtlib"
#endif

// IVL test_only()

static_assert(ivl::fmt::detail::has_formatter<std::stacktrace, char>());

void use() {
  ivl::fmt_raw::formatter<std::stacktrace> fmt;
  (void)ivl::fmt::format("{}", std::stacktrace::current());
}
