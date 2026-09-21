#define IVL_FMT_USE_FMT_MANUAL
#include "utility"
#include <vector>

#ifndef IVL_FMT_VIA_FMT_MANUAL
#error "expected formatting via fmtlib wrappers"
#endif

// IVL test_only()

int foo();

static_assert(ivl::fmt::format("{}", ^^foo) == "foo");
static_assert(ivl::fmt::format("{:?}", ^^foo) == "int foo()");
static_assert(ivl::fmt::format("{::?}", ^^foo) == "\"foo\"");
static_assert(ivl::fmt::format("{:?:?}", ^^foo) == "\"int foo()\"");

struct A;
struct B;
struct C;

static_assert(ivl::fmt::format("{::?:^7?}", std::vector{^^A, ^^B, ^^C}) == "[  \"A\"  ,   \"B\"  ,   \"C\"  ]");
