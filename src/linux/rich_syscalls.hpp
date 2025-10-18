#pragma once

#include <ivl/linux/file_descriptor>
#include <ivl/linux/kernel_result>
#include <ivl/linux/raw_syscalls>
#include <ivl/meta>
#include <cstdint>
#include <utility>

namespace ivl::linux::rich {

  // TODO: figure out how correct this is wrt. exceptions
  // compare to std::variant valueless_by_exception

  // TODO: private stuff

  // This is basically a std::variant<std::monostate, T, std::unique_ptr<Errors>...>.
  // TODO: more info
  template <typename T, typename... Errors>
    requires(std::is_standard_layout_v<T> && meta::is_unique_v<T, std::monostate, Errors...> && sizeof...(Errors) <= 64)
  struct or_rich_error {
    using raw_type     = kernel_result_detail::try_align<long, T>::left_wrapper;
    using wrapped_type = kernel_result_detail::try_align<long, T>::right_wrapper;

    // All non-negative values are T.
    // Errors are heap allocated, and the pointers are encoded into negative space.
    // Encoded nullptr represents empty state.
    union {
      raw_type     raw;
      wrapped_type wrapped;
    };

    // Due to standard-layout shenanigans this is always okay.
    long get_raw() const { return raw.get(); }

    inline static constexpr uint64_t POINTER_BITS = 57;
    inline static constexpr uint64_t POINTER_MASK = (1ULL << POINTER_BITS) - 1;

    static long encode(void* ptr, uint8_t tag) {
      // Top bit is set to 1.
      // Next 6 bits encode the tag.
      // The rest match the pointer.
      // On level 5 paging this is safe.
      // https://kib.kiev.ua/x86docs/Intel/SDMs/253668-087.pdf
      return static_cast<long>(
        (reinterpret_cast<uint64_t>(ptr) & POINTER_MASK) | (static_cast<uint64_t>(tag) << POINTER_BITS) | (1ULL << 63)
      );
    }

    static void* decode_ptr(long enc) { return reinterpret_cast<void*>(enc >> 7 << 7); }

    static uint8_t decode_tag(long enc) {
      return static_cast<uint8_t>((static_cast<uint64_t>(enc) ^ (1ULL << 63)) >> POINTER_BITS);
    }

    bool is_success() const { return get_raw() >= 0; }
    bool is_empty() const { return get_raw() == encode(nullptr, 0); }
    bool is_error() const { return get_raw() < 0 && !is_empty(); }

    void clear() { *this = or_rich_error{}; }

    // TODO: normal constructors

    // TODO: probably shouldn't go through generic tag, just have a specific rich::tag
    explicit or_rich_error(meta::tag<std::monostate>, auto&&...) : raw(encode(nullptr, 0)) {}
    explicit or_rich_error(meta::tag<T>, auto&&... args) : wrapped(FWD(args)...) {}
    template <meta::same_as_one_of<Errors...> E>
    explicit or_rich_error(meta::tag<E>, auto&&... args) {
      auto ptr = new E(FWD(args)...);

      uint8_t tag = 0;
      template for (constexpr auto error_tag : {meta::tag<Errors>{}...}) {
        if (std::same_as<E, typename decltype(error_tag)::type>) break;
        ++tag;
      }

      std::construct_at(&raw, encode(ptr, tag));
    }

    explicit or_rich_error() : or_rich_error(meta::tag<std::monostate>{}) {}

    explicit or_rich_error(long value, auto&&... args)
      requires(sizeof...(Errors) == 1)
    {
      using E = typename meta::tag<Errors...>::type;
      if (value >= 0) [[likely]] {
        std::construct_at(&wrapped, value);
      } else {
        std::construct_at(&raw, encode(new E(value, FWD(args)...), 0));
      }
    }

    or_rich_error(const or_rich_error&)            = delete;
    or_rich_error& operator=(const or_rich_error&) = delete;

    or_rich_error(or_rich_error&& o) {
      if (o.is_success()) {
        std::construct_at(&wrapped, std::move(o).wrapped.get());
        // TODO: this leaves o in T moved-from state, should it move into empty?
        // changed to empty
        // but still TODO, think about all this
        std::destroy_at(&o.wrapped);
        std::construct_at(&o.raw, encode(nullptr, 0));
      } else {
        std::construct_at(&raw, o.get_raw());
        o.raw.get() = encode(nullptr, 0);
      }
    }

    or_rich_error& operator=(or_rich_error&& o) {
      if (this == &o) return *this;
      // TODO
      if (is_success() && o.is_success()) {
        std::swap(wrapped, o.wrapped);
      } else if (!is_success() && !o.is_success()) {
        std::swap(raw, o.raw);
      } else if (is_success() && !o.is_success()) {
        auto tmp = o.raw.get();
        std::construct_at(&o.wrapped, std::move(wrapped));
        std::destroy_at(&wrapped);
        std::construct_at(&o.raw, tmp);
      } else /* !is_success() && o.is_success() */ {
        auto tmp = raw.get();
        std::construct_at(&wrapped, std::move(o.wrapped));
        // TODO: destruction might be unnecessary
        std::destroy_at(&o.wrapped);
        std::construct_at(&raw, tmp);
      }
      return *this;
    }

    // TODO: might be doable via visit()
    ~or_rich_error() {
      if (is_empty()) return;
      if (is_success()) {
        std::destroy_at(&wrapped);
        return;
      }

      void*   ptr         = decode_ptr(get_raw());
      uint8_t tag         = decode_tag(get_raw());
      uint8_t error_index = 0;

      template for (constexpr auto error_tag : {meta::tag<Errors>{}...}) {
        if (error_index == tag) {
          delete static_cast<typename decltype(error_tag)::type*>(ptr);
          return;
        }
        ++error_index;
      }

      std::unreachable();
    }

    decltype(auto) visit(this auto&& self, auto&& callable) {
      if (self.is_success()) {
        return FWD(callable)(FWD(self).wrapped.get());
      }
      if (self.is_empty()) {
        // TODO: not same value category, do I care?
        return FWD(callable)(std::monostate{});
      }

      void*   ptr         = decode_ptr(self.get_raw());
      uint8_t tag         = decode_tag(self.get_raw());
      uint8_t error_index = 0;

      template for (constexpr auto error_tag : {meta::tag<Errors>{}...}) {
        if (error_index == tag) {
          return FWD(callable)(std::forward_like<decltype(self)>(*static_cast<typename decltype(error_tag)::type*>(ptr)));
        }
        ++error_index;
      }

      std::unreachable();
    }

    decltype(auto) unwrap_or_throw(this auto&& self) {
      if (self.is_success()) [[likely]]
        return FWD(self).wrapped.get();

      if (self.is_empty()) throw std::monostate{};

      void*   ptr         = decode_ptr(self.get_raw());
      uint8_t tag         = decode_tag(self.get_raw());
      uint8_t error_index = 0;

      template for (constexpr auto error_tag : {meta::tag<Errors>{}...}) {
        if (error_index == tag) {
          // Moves even from a const& or_rich_error.
          // TODO: it's already on heap, is it posible to avoid exception allocation?
          // If yes, it would allow setting self to empty, making destruction trivial.
          throw std::move(*static_cast<typename decltype(error_tag)::type*>(ptr));
        }
        ++error_index;
      }

      std::unreachable();
    }
  };

  struct open_error {
    long        error;
    std::string filename;
    int         flags;
    mode_t      mode;
  };

  or_rich_error<wide_owned_file_descriptor, open_error> open(const char* filename, int flags, mode_t mode) {
    return or_rich_error<wide_owned_file_descriptor, open_error>(
      raw_syscalls::open(filename, flags, mode), filename, flags, mode
    );
  }

} // namespace ivl::linux::rich
