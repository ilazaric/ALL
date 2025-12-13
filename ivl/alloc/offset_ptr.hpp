#pragma once

#include <cstdint>

namespace ivl::alloc {

  template <typename T, std::uintptr_t Offset, typename OffsetType = std::uint32_t>
  struct offset_ptr {
  private:
    OffsetType offset;

  public:
    using element_type = T;
    // TODO: this was shamelessly taken from boost::interprocess::offset_ptr, not
    // sure why needed
    using value_type = std::remove_cv_t<T>;
    using pointer    = T*;
    using reference  = T&;
    // TODO: check if sufficiently morally bankrupt
    using difference_type   = std::make_signed_t<OffsetType>;
    using iterator_category = std::contiguous_iterator_tag;

  private:
    offset_ptr(OffsetType offset) noexcept : offset(offset) {}

  public:
    offset_ptr() = default;

    offset_ptr(std::nullptr_t) noexcept : offset(0) {}

    // // T -> const T
    // // void -> const void
    // offset_ptr(offset_ptr<std::remove_const_t<T>, Offset, OffsetType> o)
    // noexcept
    //   requires(std::is_const_v<T>)
    //   : offset(o.offset){}

    // // T -> void
    // // T -> const void
    // template<typename U>
    // offset_ptr(offset_ptr<U, Offset, OffsetType> o) noexcept
    //   requires(std::is_void_v<T> && !std::is_const_v<U>)
    //   : offset(o.offset){}

    // // const T -> const void
    // template<typename U>
    // offset_ptr(offset_ptr<const U, Offset, OffsetType> o) noexcept
    //   requires(std::is_void_v<T> && std::is_const_v<T> && !std::is_void_v<U>)
    //   : offset(o.offset){}

    // // static_cast<Tp>(Vp)
    // explicit offset_ptr(offset_ptr<void, Offset, OffsetType> o) noexcept
    //   requires(!std::is_void_v<T> && !std::is_const_v<T>)
    //   : offset(o.offset){}

    // // static_cast<CTp>(CVp)
    // explicit offset_ptr(offset_ptr<const void, Offset, OffsetType> o) noexcept
    //   requires(!std::is_void_v<T> && std::is_const_v<T>)
    //   : offset(o.offset){}

    template <typename U>
    // prevents int -> char
      requires(std::is_same_v<std::remove_const_t<T>, std::remove_const<U>> || std::is_void_v<T> || std::is_void_v<U>)
    explicit(std::is_void_v<U> && !std::is_void_v<T>) offset_ptr(offset_ptr<U, Offset, OffsetType> o) noexcept
      requires(!std::is_same_v<T, U> && (std::is_const_v<T> || !std::is_const_v<U>))
        : offset(o.offset) {}

    offset_ptr(const offset_ptr&) = default;
    offset_ptr(offset_ptr&&)      = default;

    offset_ptr& operator=(std::nullptr_t) noexcept {
      offset = 0;
      return *this;
    }

    offset_ptr& operator=(const offset_ptr&) = default;
    offset_ptr& operator=(offset_ptr&&)      = default;

    ~offset_ptr() = default;

    friend auto operator<=>(offset_ptr, offset_ptr) = default;

    reference operator*() const
      requires(!std::is_void_v<element_type>)
    {
      std::uintptr_t intptr  = static_cast<std::uintptr_t>(offset) + Offset;
      void*          voidptr = reinterpret_cast<void*>(intptr);
      T*             ptr     = static_cast<T*>(voidptr);
      // TODO: check if good
      return *std::launder(ptr);
    }

    pointer operator->() const
      requires(!std::is_void_v<element_type>)
    {
      return &**this;
    }

    offset_ptr& operator++()
      requires(!std::is_void_v<element_type>)
    {
      offset += sizeof(T);
      return *this;
    }

    offset_ptr& operator--()
      requires(!std::is_void_v<element_type>)
    {
      offset -= sizeof(T);
      return *this;
    }

    offset_ptr operator++(int)
      requires(!std::is_void_v<element_type>)
    {
      auto ret = *this;
      offset += sizeof(T);
      return ret;
    }

    offset_ptr operator--(int)
      requires(!std::is_void_v<element_type>)
    {
      auto ret = *this;
      offset -= sizeof(T);
      return ret;
    }

    offset_ptr& operator+=(difference_type delta)
      requires(!std::is_void_v<element_type>)
    {
      offset += static_cast<OffsetType>(delta);
      return *this;
    }

    friend offset_ptr operator+(offset_ptr ptr, difference_type delta)
      requires(!std::is_void_v<element_type>)
    {
      ptr += delta;
      return ptr;
    }

    friend offset_ptr operator+(difference_type delta, offset_ptr ptr)
      requires(!std::is_void_v<element_type>)
    {
      ptr += delta;
      return ptr;
    }

    offset_ptr& operator-=(difference_type delta)
      requires(!std::is_void_v<element_type>)
    {
      offset -= static_cast<OffsetType>(delta);
      return *this;
    }

    friend offset_ptr operator-(offset_ptr ptr, difference_type delta)
      requires(!std::is_void_v<element_type>)
    {
      ptr -= delta;
      return ptr;
    }

    difference_type operator-(offset_ptr ptr) const
      requires(!std::is_void_v<element_type>)
    {
      return static_cast<difference_type>(offset - ptr.offset);
    }

    reference operator[](difference_type delta) const
      requires(!std::is_void_v<element_type>)
    {
      return *(*this + delta);
    }

    // operator offset_ptr<const T, Offset, OffsetType>() const
    //   requires(!std::is_void_v<element_type> &&
    //   !std::is_const_v<element_type>){ return offset_ptr<const T, Offset,
    //   OffsetType>(offset);
    // }

    // operator offset_ptr<void, Offset, OffsetType>() const
    //   requires(!std::is_void_v<element_type> &&
    //   !std::is_const_v<element_type>){ return offset_ptr<void, Offset,
    //   OffsetType>(offset);
    // }

    // operator offset_ptr<const void, Offset, OffsetType>() const
    //   requires(!std::is_same_v<element_type, const void>){
    //   return offset_ptr<const void, Offset, OffsetType>(offset);
    // }

    explicit operator bool() const noexcept { return offset != 0; }

    static offset_ptr pointer_to(element_type& el)
      requires(!std::is_void_v<element_type>)
    {
      return offset_ptr(reinterpret_cast<std::uintptr_t>(&el) - Offset);
    }
  };

} // namespace ivl::alloc
