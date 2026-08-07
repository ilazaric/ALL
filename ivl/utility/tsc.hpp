#pragma once

#if !defined(__x86_64__)
#error "This header only works for x86-64"
#endif

#include <compare>
#include <cstdint>
#include <immintrin.h>

namespace ivl {
struct tsc_time_point {
  int64_t value;
  auto operator<=>(const tsc_time_point&) const = default;
};

struct tsc_duration {
  int64_t value;
  auto operator<=>(const tsc_duration&) const = default;
};

tsc_time_point tsc_now() { return tsc_time_point{(int64_t)_rdtsc()}; }

tsc_duration operator+(tsc_duration a, tsc_duration b) { return {a.value + b.value}; }
tsc_duration operator-(tsc_duration a, tsc_duration b) { return {a.value - b.value}; }
tsc_duration& operator+=(tsc_duration& a, tsc_duration b) {
  a.value += b.value;
  return a;
}
tsc_duration& operator-=(tsc_duration& a, tsc_duration b) {
  a.value -= b.value;
  return a;
}

tsc_duration operator-(tsc_time_point a, tsc_time_point b) { return {a.value - b.value}; }

tsc_time_point operator+(tsc_time_point a, tsc_duration b) { return {a.value + b.value}; }
tsc_time_point operator+(tsc_duration a, tsc_time_point b) { return {a.value + b.value}; }
tsc_time_point operator-(tsc_time_point a, tsc_duration b) { return {a.value - b.value}; }

tsc_time_point& operator+=(tsc_time_point& a, tsc_duration b) {
  a.value += b.value;
  return a;
}
tsc_time_point& operator-=(tsc_time_point& a, tsc_duration b) {
  a.value -= b.value;
  return a;
}
} // namespace ivl
