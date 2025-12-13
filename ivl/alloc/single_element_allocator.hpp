#pragma once

#include "mmap_fixed_storage.hpp"

namespace ivl::alloc {

  namespace sea_detail {

    template <typename T, std::uintptr_t Location, std::size_t Size>
    union Slot {
      T             t;
      std::uint32_t nxt;

      Slot() {}
      Slot(std::uint32_t nxt) : nxt(nxt) {}
      ~Slot() {}
    };

    template <typename T, std::uintptr_t Location, std::size_t Size>
    ivl::alloc::MmapFixedStorage<Location, Size * sizeof(T)> storage;

    template <typename T, std::uintptr_t Location, std::size_t Size>
    Slot<T, Location, Size>* get_slot(std::uint32_t idx) {
      (void)storage<T, Location, Size>;
      return reinterpret_cast<Slot<T, Location, Size>*>(Location + (std::size_t)idx * sizeof(T));
    }

    template <typename T, std::uintptr_t Location, std::size_t Size>
    struct Pointer {
      std::uint32_t idx;

      Pointer() = default;
      Pointer(std::nullptr_t) : idx(0) {}
      Pointer(std::uint32_t idx) : idx(idx) {}

      Pointer(const Pointer&) = default;
      Pointer(Pointer&&)      = default;

      Pointer& operator=(const Pointer&) = default;
      Pointer& operator=(Pointer&&)      = default;

      ~Pointer() = default;

      T& operator*() { return get_slot<T, Location, Size>(idx)->t; }
      T* operator->() { return &**this; }

      bool operator==(const Pointer&) const = default;

      operator bool() const { return idx != 0; }
    };

  } // namespace sea_detail

  // an allocator that only allocates one element at a time, backed by mmap
  // Size includes the null address, so this gives exactly Size-1 actual allocations
  template <typename T, std::uintptr_t Location, std::size_t Size>
    requires(Size <= (1ULL << 32))
  struct SingleElementAllocator {
    inline static std::size_t last_idx = 1;

    // zeroth slot is used as a linked list to free nodes
    // this initializes it (and starts lifetime, so no UB)
    struct Initializer {
      Initializer() { std::construct_at(get_slot<T>(0), 0); }
    };

    static Initializer initializer;

    // lambda breaks for some reason on incomplete types, TODO: investigate
    // inline static const auto initializer = []{
    //   std::construct_at(get_slot<T>(0), 0);
    //   return 0;
    // }();

    using pointer         = sea_detail::Pointer<T, Location, Size>;
    using value_type      = T;
    using size_type       = std::uint32_t;
    using difference_type = std::int32_t;

    static auto get_slot(std::uint32_t idx) { return sea_detail::get_slot<T, Location, Size>(idx); }

    static pointer allocate() {
      (void)initializer;
      if (get_slot(0)->nxt) {
        auto ret         = get_slot(0)->nxt;
        get_slot(0)->nxt = get_slot(ret)->nxt;
        return pointer(ret);
      } else {
        std::construct_at(get_slot(last_idx));
        return pointer(last_idx++);
      }
    }

    static void deallocate(pointer p) {
      get_slot(p.idx)->nxt = get_slot(get_slot(0)->nxt)->nxt;
      get_slot(0)->nxt     = p.idx;
    }

    // this must be used in std::unique_ptrs
    // std::unique_ptr<T, Deleter>
    struct Deleter {
      using pointer = sea_detail::Pointer<T, Location, Size>;
      void operator()(pointer p) {
        std::destroy_at(&*p);
        deallocate(p);
      }
    };
  };

} // namespace ivl::alloc
