#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <new>
#include <type_traits>

namespace ivl::bump {

  struct BumpUpAllocRange {
    std::byte* low;
    std::byte* high;
  };

  template <size_t Size>
  struct BumpUpAllocOwning : BumpUpAllocRange {
    alignas(alignof(std::max_align_t)) std::byte memory[Size];

    BumpUpAllocOwning() : BumpUpAllocRange(memory, memory + Size) {}

    BumpUpAllocOwning(const BumpUpAllocOwning&) = delete;
    BumpUpAllocOwning(BumpUpAllocOwning&&)      = delete;

    BumpUpAllocOwning& operator=(const BumpUpAllocOwning&) = delete;
    BumpUpAllocOwning& operator=(BumpUpAllocOwning&&)      = delete;

    ~BumpUpAllocOwning() = default;

    void reset() {
      low  = memory;
      high = memory + Size;
    }
  };

  template <typename T>
  struct BumpUpAlloc {
    BumpUpAllocRange* range;

    using value_type = T;

    BumpUpAlloc(BumpUpAllocRange* range) : range(range) {}

    template <size_t Size>
    BumpUpAlloc(BumpUpAllocOwning<Size>& owner) : range(&owner) {}

    template <typename U>
    BumpUpAlloc(const BumpUpAlloc<U>& o) : range(o.range) {}

    T* allocate(size_t n) {
      auto low  = reinterpret_cast<uintptr_t>(range->low);
      auto high = reinterpret_cast<uintptr_t>(range->high);

      // std::cout << "allocator: " << low << " " << high << " " << n << " " << sizeof(T) <<
      // std::endl;

      // low becomes smallest number >= low that is divisible by alignof(T)
      low = ((low - 1) & ~(alignof(T) - 1)) + alignof(T);

      if (low + sizeof(T) > high) throw std::bad_alloc();

      range->low = reinterpret_cast<std::byte*>(low + sizeof(T) * n);
      return reinterpret_cast<T*>(low);
    }

    void deallocate(T*, size_t) {}
  };

} // namespace ivl::bump
