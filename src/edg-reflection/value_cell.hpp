#pragma once

#include "memory_cell.hpp"

namespace ivl::refl {

  // special common case of `MemoryCell`
  // where we store/load values of type T
  template <typename T, typename = decltype([] {})>
  struct ValueCell {
  private:
    static constexpr MemoryCell<> cell;

  public:
    static consteval T load() { return std::meta::extract<T>(cell.load()); }

    static consteval void store(T t) { return cell.store(std::meta::reflect_value<T>(std::move(t))); }
  };

} // namespace ivl::refl
