#include <ivl/logger>
#include "cpuid"
#include <ivl/format>

constexpr uint64_t MASK = (1ULL << 16) - 1;
constexpr uint64_t EAX = 1ULL << 0;
constexpr uint64_t EBX = 1ULL << 16;
constexpr uint64_t ECX = 1ULL << 32;
constexpr uint64_t EDX = 1ULL << 48;

constexpr uint64_t DEC = 0;
constexpr uint64_t HEX = 1;
constexpr uint64_t STR = 2;

void print_reg(unsigned int reg, uint64_t ctl) {
  if (ctl == DEC) ivl::fmt::print("{}", reg);
  else if (ctl == HEX) ivl::fmt::print("{:#x}", reg);
  else if (ctl == STR) ivl::fmt::print("{:?}", std::string_view((const char*)&reg, (const char*)(&reg + 1)));
  else {
    ivl::fmt::println(stderr, "unrecognized ctl: {}", ctl);
    std::terminate();
  }
}

void cpuid_log(unsigned int op, int count = 0, uint64_t ctl = 0) {
  auto ret = ivl::raw_cpuid(op, count);
  ivl::fmt::print("op={:#x}", op);
  if (count) ivl::fmt::print(" count={:#x}", count);
  ivl::fmt::print(" eax=");
  print_reg(ret.a, (ctl / EAX) & MASK);
  ivl::fmt::print(" ebx=");
  print_reg(ret.b, (ctl / EBX) & MASK);
  ivl::fmt::print(" ecx=");
  print_reg(ret.c, (ctl / ECX) & MASK);
  ivl::fmt::print(" edx=");
  print_reg(ret.d, (ctl / EDX) & MASK);
  ivl::fmt::println("");
}

auto carve(unsigned int num, auto... widths) -> std::array<unsigned int, sizeof...(widths)> {
  contract_assert((0 + ... + widths) == 32);
  auto lambda = [&](unsigned int width) {
    auto ret = num ^ (num >> width << width);
    num >>= width;
    return ret;
  };
  return {lambda(widths)...};
}

int ivl_main() {
  cpuid_log(0, 0, (EBX | ECX | EDX) * STR);
  return 0;
}
