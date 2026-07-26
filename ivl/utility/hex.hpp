#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ivl::util {
inline std::string hex(std::string_view sv) {
  std::string ret(sv.size() * 2, '\0');
  auto hexc = [](int x) -> char { return x < 10 ? '0' + x : 'a' + x - 10; };
  for (size_t i = 0; i < sv.size(); ++i) {
    ret[2 * i] = hexc(((unsigned char)sv[i]) / 16);
    ret[2 * i + 1] = hexc(((unsigned char)sv[i]) % 16);
  }
  return ret;
}

inline std::string unhex(std::string_view sv) {
  contract_assert(sv.size() % 2 == 0);
  std::string ret(sv.size() / 2, '\0');
  auto unhexc = [](char x) -> uint32_t {
    if (x >= '0' && x <= '9') return x - '0';
    if (x >= 'a' && x <= 'f') return (x - 'a') + 10;
    contract_assert(false);
    return 0;
  };
  for (size_t i = 0; i < ret.size(); ++i) {
    ret[i] = unhexc(sv[i * 2]) * 16 + unhexc(sv[i * 2 + 1]);
  }
  return ret;
}
} // namespace ivl::util
