#pragma once

template <auto Value>
struct ConstexprWrapper {
  static constexpr auto value = Value;
  constexpr             operator decltype(Value)() const noexcept { return value; };
};
