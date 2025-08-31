#pragma once

#include <array>
#include <cassert>
#include <cstddef>

#include "segment_tree2.hpp"

#include <ivl/debug>

namespace ivl::alloc {

  /*
    storage needs a data() and size() fn
   */

  namespace spa_detail {

    template <typename T, typename Traits>
    struct Pointer {
      static_assert(!std::is_volatile_v<T>);
      static_assert(!std::is_reference_v<T>);

      // pointer_traits
      using element_type    = T;
      using size_type       = std::uint32_t;
      using difference_type = std::int32_t;

      template <typename U>
      using rebind = Pointer<U, Traits>;

      // TODO: pointer_to
      // should be done, check

      // iterator_traits
      using value_type = std::remove_cv_t<T>;
      // hate this
      // TODO: think even more about this
      using pointer = T*;
      // using pointer = Pointer;
      // TODO: should this be a proxy that is convertible to T& ?
      // contiguous_iterator concept seems to indicate "NO"
      using reference         = std::add_lvalue_reference_t<std::conditional_t<std::is_void_v<T>, int, T>>;
      using iterator_category = std::random_access_iterator_tag;
      using iterator_concept  = std::contiguous_iterator_tag;

      static Pointer pointer_to(reference r) {
        auto    ri = reinterpret_cast<std::uintptr_t>(&r);
        Pointer out{};
        auto    zi = reinterpret_cast<std::uintptr_t>(Traits::storage.data());
        IVL_DBG_ASSERT(
          ri >= zi && ri < zi + Traits::storage.size() && "bad reference, not from our alloc", zi, ri,
          zi + Traits::storage.size(), &r
        );
        auto delta = ri - zi;
        out.offset += delta;
        return out;
      }

      static constexpr bool isConst = std::is_const_v<T>;
      static constexpr bool isVoid  = std::is_void_v<T>;

      std::uint32_t offset;

      // TODO: check if needed
      // Pointer() : offset(0){}
      Pointer() = default;
      Pointer(std::nullptr_t) : offset(0) {}

      template <typename U>
        requires(!std::is_same_v<T, U> && isConst >= std::is_const_v<U> && isVoid >= std::is_void_v<U>)
      Pointer(Pointer<U, Traits> p) : offset(p.offset) {}

      // oof :'(
      template <std::same_as<void> U>
        requires(!isVoid)
      explicit Pointer(Pointer<U, Traits> p) : offset(p.offset) {}
      explicit Pointer(Pointer<const void, Traits> p)
        requires(isConst && !isVoid)
          : offset(p.offset) {}

      explicit operator bool() const { return offset != 0; }

      // TODO: stuff noexcept and constexpr all over the place
      auto operator<=>(const Pointer&) const = default;
      bool operator==(const Pointer&) const  = default;

      Pointer& operator++() {
        offset += sizeof(T);
        return *this;
      }
      Pointer operator++(int) {
        Pointer cpy = *this;
        ++*this;
        return cpy;
      }
      Pointer& operator--() {
        offset -= sizeof(T);
        return *this;
      }
      Pointer operator==(int) {
        Pointer cpy = *this;
        --*this;
        return cpy;
      }
      Pointer& operator+=(difference_type n) {
        offset += n * sizeof(T);
        return *this;
      }
      friend Pointer operator+(Pointer p, difference_type n) {
        p += n;
        return p;
      }
      friend Pointer operator+(difference_type n, Pointer p) {
        p += n;
        return p;
      }
      Pointer& operator-=(difference_type n) {
        offset -= n * sizeof(T);
        return *this;
      }
      friend Pointer operator-(Pointer p, difference_type n) {
        p -= n;
        return p;
      }
      friend difference_type operator-(Pointer p, Pointer q) {
        return (difference_type)(p.offset - q.offset) / sizeof(T);
      }
      reference operator[](difference_type n) const { return *(*this + n); }

      // TODO: pretty sure smth is broken with nullptr, explore

      // TODO: should i cast to char* just in case?
      reference operator*() const { return *reinterpret_cast<T*>(Traits::storage.data() + offset); }
      // seems that `pointer{}.operator->()` does not actually have
      // to be equal to `(T*)nullptr`, haven't seen anything in the standard
      T* operator->() const { return /*offset == 0 ? nullptr :*/ &**this; }
    };

    template <typename T, typename Traits>
    std::ostream& operator<<(std::ostream& out, const Pointer<T, Traits>& p) {
      return out << p.offset;
    }

    template <typename Traits>
    struct Allocator {
      using pointer = Pointer<void, Traits>;

      // -1 bc first chunk is not used bc nullptr lives there
      using segment_tree_type = SegmentTree2<Traits::storage.size() / Traits::segment_tree_chunk_size>;

      static std::uint32_t chunk_count(std::uint32_t n) {
        return (n + Traits::segment_tree_chunk_size - 1) / Traits::segment_tree_chunk_size;
      }

      // it turns out i prefer shoving metadata into the storage
      // bc having a static variable bloats the executable size
      // added benefit, it occupies nullptr slot
      static segment_tree_type& initialize_segment_tree() {
        static_assert(sizeof(segment_tree_type) < Traits::storage.size());
        // xD i suppose
        auto ptr = std::construct_at<segment_tree_type>(
          static_cast<segment_tree_type*>(static_cast<void*>(Traits::storage.data()))
        );
        // this is kinda cute tbh
        ptr->take(chunk_count(sizeof(segment_tree_type)));
        return *ptr;
      }

      inline static segment_tree_type& segment_tree = initialize_segment_tree();

      static pointer segment_tree_allocate(std::uint32_t n) {
        auto alloc = segment_tree.take(chunk_count(n));
        if (alloc == segment_tree_type::FAILURE) [[unlikely]] {
          throw std::bad_alloc{};
        }
        pointer out;
        out.offset = alloc * Traits::segment_tree_chunk_size;
        return out;
      }

      static void segment_tree_deallocate(pointer p, std::uint32_t n) {
        auto x = p.offset / Traits::segment_tree_chunk_size;
        segment_tree.give(x, chunk_count(n));
      }

      inline static std::array<pointer, Traits::free_list_limit + 1> free_list_heads{};

      static pointer free_list_allocate(std::uint32_t n) {
        pointer& head = free_list_heads[n];
        if (!head) [[unlikely]] {
          auto ptr = segment_tree_allocate(n * Traits::free_list_steal_coef);
          for (std::uint32_t i = 0; i < Traits::free_list_steal_coef; ++i) {
            auto newhead = static_cast<Pointer<pointer, Traits>>(ptr);
            ptr.offset += n;
            std::construct_at(&*newhead, head);
            head = newhead;
          }
        }

        auto cpy = head;
        // this looks hilarious
        head = *static_cast<Pointer<pointer, Traits>>(cpy);
        std::destroy_at(&*static_cast<Pointer<pointer, Traits>>(cpy));
        return cpy;
      }

      static void free_list_deallocate(pointer p, std::uint32_t n) {
        auto& head    = free_list_heads[n];
        auto  newhead = static_cast<Pointer<pointer, Traits>>(p);
        std::construct_at(&*newhead, head);
        head = newhead;
      }

      static std::uint32_t align32(std::uint32_t n) { return (n + 3u) & ~3u; }

      static pointer allocate(std::uint32_t n) {
        n = align32(n);
        // TODO
        if (n <= Traits::free_list_limit) [[likely]] {
          return free_list_allocate(n);
        } else {
          return segment_tree_allocate(n);
        }
      }

      static void deallocate(pointer p, std::uint32_t n) {
        n = align32(n);
        // TODO
        if (n <= Traits::free_list_limit) [[likely]] {
          free_list_deallocate(p, n);
        } else {
          segment_tree_deallocate(p, n);
        }
      }
    };

  } // namespace spa_detail

  template <typename T, typename Traits>
  struct SmallPtrAllocator {
    // TODO
    using pointer            = spa_detail::Pointer<T, Traits>;
    using const_pointer      = spa_detail::Pointer<const T, Traits>;
    using void_pointer       = spa_detail::Pointer<void, Traits>;
    using const_void_pointer = spa_detail::Pointer<const void, Traits>;

    using value_type = T;

    using size_type       = std::uint32_t;
    using difference_type = std::int32_t;

    template <typename U>
    struct rebind {
      using other = SmallPtrAllocator<U, Traits>;
    };

    static pointer allocate(std::uint32_t n) {
      return (n, static_cast<pointer>(spa_detail::Allocator<Traits>::allocate(n * sizeof(T))));
    }

    static void deallocate(pointer p, std::uint32_t n) {
      // LOG(p, n);
      spa_detail::Allocator<Traits>::deallocate(static_cast<void_pointer>(p), n * sizeof(T));
    }
  };

} // namespace ivl::alloc
