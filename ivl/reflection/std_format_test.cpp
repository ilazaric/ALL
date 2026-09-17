#define IVL_FMT_USE_STD
#include "utility"

// IVL test_only()

int foo();

static_assert(ivl::fmt::format("{}", ^^foo) == "foo");
static_assert(ivl::fmt::format("{:?}", ^^foo) == "int foo()");
static_assert(ivl::fmt::format("{::?}", ^^foo) == "\"foo\"");
static_assert(ivl::fmt::format("{:?:?}", ^^foo) == "\"int foo()\"");
