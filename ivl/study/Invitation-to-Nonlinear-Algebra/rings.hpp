#pragma once

#include <ivl/format>
#include "concepts"
#include <gmpxx.h>
#include <stdexcept>

using Z = mpz_class;

using Q = mpq_class;

template<>
struct is_field_decl<Q> : std::true_type {};

template<>
struct ivl::fmt_raw::formatter<Z> {
  constexpr auto parse(auto& ctx) {
    if (ctx.begin() == ctx.end() || *ctx.begin() == '}') return ctx.begin();
    throw std::runtime_error("invalid format specifier");
  }
  auto format(const Z& z, auto& ctx) const { return ivl::fmt::format_to(ctx.out(), "{}", z.get_str()); }
};

template<>
struct ivl::fmt_raw::formatter<Q> {
  constexpr auto parse(auto& ctx) {
    if (ctx.begin() == ctx.end() || *ctx.begin() == '}') return ctx.begin();
    throw std::runtime_error("invalid format specifier");
  }
  auto format(const Q& q, auto& ctx) const { return ivl::fmt::format_to(ctx.out(), "{}", q.get_str()); }
};
