#pragma once

#include <cstddef>

namespace ivl::literals::ints_exact {

  constexpr std::uint32_t operator""_u32(unsigned long long int arg){
    return static_cast<std::uint32_t>(arg);
  }

  constexpr std::int32_t operator""_i32(unsigned long long int arg){
    return static_cast<std::int32_t>(arg);
  }

  constexpr std::uint64_t operator""_u64(unsigned long long int arg){
    return static_cast<std::uint64_t>(arg);
  }

  constexpr std::int64_t operator""_i64(unsigned long long int arg){
    return static_cast<std::int64_t>(arg);
  }

} // namespace ivl::literals::ints_exact
