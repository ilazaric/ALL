#pragma once

// TODO: probably should make ivl/arch/{x86_64,...}/ and move this there

#if !defined(__x86_64__)
#error "This header only works for x86-64"
#endif

#include <cstdint>

namespace ivl {
struct cpuid_output {
  uint32_t a, b, c, d;
};

// TODO: take a look at asm of this, looks really weird
cpuid_output raw_cpuid(uint32_t leaf, uint32_t subleaf = 0) {
  uint32_t a asm("eax") = leaf;
  uint32_t b asm("ebx");
  uint32_t c asm("ecx") = subleaf;
  uint32_t d asm("edx");
  asm("cpuid" : "+a"(a), "=b"(b), "+c"(c), "=d"(d));
  return cpuid_output{
    .a = a,
    .b = b,
    .c = c,
    .d = d,
  };
}
} // namespace ivl
