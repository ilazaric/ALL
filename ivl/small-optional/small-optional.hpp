#pragma once

namespace ivl {

  template <typename T>
  struct DefaultTombstoner {
    static bool is_tombstone(void* ptr) { return *static_cast<T*>(ptr) == T{}; }

    static void tombstoneify(void* ptr) { new (ptr) T; }

    static void detombstoneify(void* ptr) { static_cast<T*>(ptr)->~T(); }
  };

  template <typename T>
  concept Tombstoney = requires(void* ptr) {
    if (T::is_tombstone(ptr))
      ;
    T::tombstoneify(ptr);
    // TODO: remove this, noop if not defined
    T::detombstoneify(ptr);
  };

  struct NullOpt_t {
    constexpr explicit NullOpt_t(int) {}
  };
  inline constexpr NullOpt{0};

  template <typename T, Tombstoney Tombstoner = DefaultTombstoner<T>>
  struct SmallOptional {
    union {
      T data;
    };

    SmallOptional() { Tombstoner::tombstoneify(&data); }

    SmallOptional(NullOpt_t) : SmallOptional() {}

    SmallOptional(const SmallOptional& o) {
      if (o) {
        new (&data) T(*o);
      } else {
        Tombstoner::tombstoneify(&data);
      }
    }

    SmallOptional(SmallOptional&& o) {
      if (o) {
        // TODO: check if *std::move(o)
        // expr-equiv to std::move(*o)
        new (&data) T(std::move(*o));
      } else {
        Tombstoner::tombstoneify(&data);
      }
    }

    // TODO
    template <typename U>
    SmallOptional(const SmallOptional<U>& o);

    // TODO
    template <typename U>
    SmallOptional(SmallOptional<U>&& o);

    // TODO
    template <typename... Args>
    explicit SmallOptional(std::in_place_t, Args&&... args);

    // TODO: other constructors

    ~SmallOptional() {
      if (has_value()) {
        data.~T();
      } else {
        Tombstoner::detombstoneify(&data);
      }
    }

    // TODO: assignments

    T&        operator*() & { return data; }
    const T&  operator*() const& { return data; }
    T&&       operator*() && { return data; }
    const T&& operator*() const&& { return data; }

    T*       operator->() { return &data; }
    const T* operator->() const { return &data; }

    bool has_value() const { return Tombstoner::is_tombstone(&data); }

    explicit operator bool() const { return has_value(); }

    T& value() & {
      if (not has_value()) throw std::bad_optional_access{};
      return data;
    }

    const T& value() const& {
      if (not has_value()) throw std::bad_optional_access{};
      return data;
    }

    T&& value() && {
      if (not has_value()) throw std::bad_optional_access{};
      return data;
    }

    const T&& value() const&& {
      if (not has_value()) throw std::bad_optional_access{};
      return data;
    }

    // TODO: value_or
  };

} // namespace ivl
